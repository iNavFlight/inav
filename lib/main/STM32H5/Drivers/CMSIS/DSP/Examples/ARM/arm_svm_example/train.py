from sklearn import svm
import random
import numpy as np
import math

from pylab import scatter,figure, clf, plot, xlabel, ylabel, xlim, ylim, title, grid, axes, show,semilogx, semilogy
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
from matplotlib.colors import BoundaryNorm,ListedColormap

# Generation of data to train the SVM classifier
# 100 vectors are generated. Vector have dimension 2 so can be represented as points
NBVECS = 100
VECDIM = 2

# A cluster of point is generated around the origin.
ballRadius = 0.5
x = ballRadius * np.random.randn(NBVECS,2)

# An annulus of point is generated around the central cluster.
angle = 2.0*math.pi * np.random.randn(1,NBVECS)
radius = 3.0+0.1*np.random.randn(1,NBVECS)

xa = np.zeros((NBVECS,2))
xa[:,0]=radius*np.cos(angle)
xa[:,1]=radius*np.sin(angle)

# All points are concatenated
X_train=np.concatenate((x,xa))

# First points (central cluster) are corresponding to class 0
# OTher points (annulus) are corresponding to class 1
Y_train=np.concatenate((np.zeros(NBVECS),np.ones(NBVECS)))

# Some bounds are computed for the graphical representation
x_min = X_train[:, 0].min()
x_max = X_train[:, 0].max()
y_min = X_train[:, 1].min()
y_max = X_train[:, 1].max()

# Training is done with a polynomial SVM
clf = svm.SVC(kernel='poly',gamma='auto', coef0=1.1)
clf.fit(X_train, Y_train)

# The classifier is tested with a first point inside first class
test1=np.array([0.4,0.1])
test1=test1.reshape(1,-1)

predicted1 = clf.predict(test1)
# Predicted class should be 0
print(predicted1)

# Second test is made with a point inside the second class (in the annulus)
test2=np.array([x_max,0]).reshape(1,-1)

predicted2 = clf.predict(test2)
# Predicted class should be 1
print(predicted2)

# The parameters of the trained classifier are printed to be used
# in CMSIS-DSP
supportShape = clf.support_vectors_.shape

nbSupportVectors=supportShape[0]
vectorDimensions=supportShape[1]

print("nbSupportVectors = %d" % nbSupportVectors)
print("vectorDimensions = %d" % vectorDimensions)
print("degree = %d" % clf.degree)
print("coef0 = %f" % clf.coef0)
print("gamma = %f" % clf._gamma)

print("intercept = %f" % clf.intercept_)

dualCoefs=clf.dual_coef_ 
dualCoefs=dualCoefs.reshape(nbSupportVectors)
supportVectors=clf.support_vectors_
supportVectors = supportVectors.reshape(nbSupportVectors*VECDIM)

print("Dual Coefs")
print(dualCoefs)

print("Support Vectors")
print(supportVectors)

# Graphical representation to display the cluster of points
# and the SVM boundary
r=plt.figure()
plt.axis('off')
XX, YY = np.mgrid[x_min:x_max:200j, y_min:y_max:200j]
Z = clf.decision_function(np.c_[XX.ravel(), YY.ravel()])

# Put the result into a color plot
Z = Z.reshape(XX.shape)

levels = MaxNLocator(nbins=15).tick_values(Z.min(), Z.max())

#cmap = plt.get_cmap('gray')
newcolors = ['#FFFFFF','#FFFFFF']
cmap = ListedColormap(newcolors)
norm = BoundaryNorm(levels, ncolors=cmap.N, clip=True)

plt.pcolormesh(XX, YY, Z > 0, cmap=cmap,norm=norm)
plt.contour(XX, YY, Z, colors=['k', 'k', 'k'],
                linestyles=['--', '-', '--'], levels=[-.5, 0, .5])

scatter(x[:,0],x[:,1],s=1.0,color='#FF6B00')
scatter(xa[:,0],xa[:,1],s=1.0,color='#95D600')

# The test points are displayed in red.
scatter(test1[:,0],test1[:,1],s=6.0,color='Red')
scatter(test2[:,0],test2[:,1],s=6.0,color='Red')
#r.savefig('fig1.jpeg')
#plt.close(r)
show()


#r=plt.figure()
#plt.axis('off')
#scatter(x[:,0],x[:,1],s=1.0,color='#FF6B00')
#scatter(xa[:,0],xa[:,1],s=1.0,color='#95D600')
#r.savefig('fig2.jpeg')
#plt.close(r)