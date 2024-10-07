from sklearn.naive_bayes import GaussianNB
import random
import numpy as np
import math

from pylab import scatter,figure, clf, plot, xlabel, ylabel, xlim, ylim, title, grid, axes, show,semilogx, semilogy
import matplotlib.pyplot as plt
from matplotlib.font_manager import FontProperties

# Generation of data to train the  classifier
# 100 vectors are generated. Vector have dimension 2 so can be represented as points
NBVECS = 100
VECDIM = 2

# 3 cluster of points are generated
ballRadius = 1.0
x1 = [1.5, 1] +  ballRadius * np.random.randn(NBVECS,VECDIM)
x2 = [-1.5, 1] + ballRadius * np.random.randn(NBVECS,VECDIM)
x3 = [0, -3] + ballRadius * np.random.randn(NBVECS,VECDIM)

# All points are concatenated
X_train=np.concatenate((x1,x2,x3))

# The classes are 0,1 and 2.
Y_train=np.concatenate((np.zeros(NBVECS),np.ones(NBVECS),2*np.ones(NBVECS)))

gnb = GaussianNB()
gnb.fit(X_train, Y_train)

print("Testing")
y_pred = gnb.predict([[1.5,1.0]])
print(y_pred)

y_pred = gnb.predict([[-1.5,1.0]])
print(y_pred)

y_pred = gnb.predict([[0,-3.0]])
print(y_pred)

# Dump of data for CMSIS-DSP

print("Parameters")
# Gaussian averages
print("Theta = ",list(np.reshape(gnb.theta_,np.size(gnb.theta_))))

# Gaussian variances
print("Sigma = ",list(np.reshape(gnb.sigma_,np.size(gnb.sigma_))))

# Class priors
print("Prior = ",list(np.reshape(gnb.class_prior_,np.size(gnb.class_prior_))))

print("Epsilon = ",gnb.epsilon_)


# Some bounds are computed for the graphical representation
x_min = X_train[:, 0].min()
x_max = X_train[:, 0].max()
y_min = X_train[:, 1].min()
y_max = X_train[:, 1].max()

font = FontProperties()
font.set_size(20)

r=plt.figure()
plt.axis('off')
plt.text(1.5,1.0,"A", verticalalignment='center', horizontalalignment='center',fontproperties=font)
plt.text(-1.5,1.0,"B",verticalalignment='center', horizontalalignment='center', fontproperties=font)
plt.text(0,-3,"C", verticalalignment='center', horizontalalignment='center',fontproperties=font)
scatter(x1[:,0],x1[:,1],s=1.0,color='#FF6B00')
scatter(x2[:,0],x2[:,1],s=1.0,color='#95D600')
scatter(x3[:,0],x3[:,1],s=1.0,color='#00C1DE')
#r.savefig('fig.jpeg')
#plt.close(r)
show()