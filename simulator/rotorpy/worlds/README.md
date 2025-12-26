# World Module

Worlds are used to represent obstacles in the map. Maps are implemented as `.json` files using the following structure: 

The world bounds are defined by the "bounds" key: 
```
{
    "bounds": {"extents": [xmin, xmax, ymin, ymax, zmin, zmax]},

```
Obstacles are defined using cuboids. The cuboids are defined as follows: 
```
    "blocks": [
        {"extents": [xmin, xmax, ymin, ymax, zmin, zmax], "color": [R, G, B]},
        ...
    ]
}
```
where the "color" value is a specified by RGB values between 0 and 1. As an example: 
```
{"extents": [0, 1, -3, 2, 0, 10], "color": [0, 1, 0]}
```
will create a box with corners (0,-3,0), (0, 2,0), (0,-3,10), (0,2,10), (1,-3,0), (1,2,0), (1,-3,10), (1,2,10). In other words, a box that has LWH dimensions (1m)x(5m)x(10m). The `[0,1,0]` label indicates it will be rendered in green. 