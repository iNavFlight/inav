
import numpy as np

def print_vec(vec, name, precise=0):
    
    mag = np.linalg.norm(vec)
    
    
    if len(vec) == 3:

        if precise == 1:
            print(f"{name:>12}: {vec[0]:>12.6f}, {vec[1]:>12.6f}, {vec[2]:>12.6f}, mag: {mag:>8.2f}")
            
        if precise == 2:
            print(f"{name:>12}: {vec[0]:>14.8f}, {vec[1]:>14.8f}, {vec[2]:>14.8f}, mag: {mag:>8.2f}")
            
        else:
            print(f"{name:>12}: {vec[0]:>10.4f}, {vec[1]:>10.4f}, {vec[2]:>10.4f}, mag: {mag:>8.2f}")
    
    elif len(vec) == 4:
        
        print(f"{name:>12}: {vec[0]:>10.4f}, {vec[1]:>10.4f}, {vec[2]:>10.4f}, {vec[3]:>10.4f}")
    
    else:
        
        raise RuntimeError(f"Unsupported vec len of {len(vec)}")
        

class NavProj:
    
    def __init__(self, lat0, lon0, alt0):
        
        self.lat0 = lat0
        self.lon0 = lon0
        self.alt0 = alt0
        
        
        self.last_lat = lat0
        self.last_lon = lon0
        
    
    def to_geodetic(self, x, y, z, type="ENU"):
       
        if type=="ENU":
            e = x
            n = y
            u = z

        elif type == "NED":
            e = y
            n = x
            u = -z
            
        else:
            raise RuntimeError(f"Unknown frame type: {type}, valid only ENU and NED.")

        # From ENU to geodetic computation
        import pymap3d

        ell = pymap3d.Ellipsoid.from_name('wgs84_mean')
        lat, lon, alt = pymap3d.enu2geodetic(e, n, u, self.lat0, self.lon0, self.alt0, ell=ell, deg=True)

        trk = self.ground_track(lat, lon)
        
        return lat, lon, alt, trk
    
    
    def ground_track(self, lat, lon):

        lat1 = np.radians(self.last_lat)
        lon1 = np.radians(self.last_lon)
        lat2 = np.radians(lat)
        lon2 = np.radians(lon)

        dlon = lon2 - lon1

        x = np.sin(dlon) * np.cos(lat2)
        y = np.cos(lat1) * np.sin(lat2) - np.sin(lat1) * np.cos(lat2) * np.cos(dlon)

        bearing = np.degrees(np.arctan2(x, y))
        
        self.last_lat = lat
        self.last_lon = lon
        
        return (bearing + 360) % 360


class InavSimState:
    
    def __init(self):
        
        self.roll = 0.0
        self.pitch = 0.0
        self.yaw = 0.0
        
        self.lat = 0.0
        self.lon = 0.0
        self.alt = 0.0
        
        self.hdg = 0.0
        self.trk = 0.0