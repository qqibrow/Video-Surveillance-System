Video-Surveillance-System
=========================

system which could provide functions including  region  detection,  human  detection,  tracking,  and  abandoned or removed object detection. I am responsible for the object tracking and detection part. 

The whole system consists of several components. The first one is background subtraction. Here we mainly refer to the code provided by Donovan Parks http://dparks.wikidot.com/. we implemented the Gaussian Mixture Model to do the backgorund subtration, but we made some improvement to overcome the illumination changing problem. As our camera is fixed, we simply learned several Gaussian Mixture Models according to the environments and recorded it. we change the models it if we detect the light change dramatically. In the changing process, we use the adaptive median filter(AMF) method(very similar to a frame-by-frame subtraction method).So the process is like:

using GMM1 -> detected light change -> change to GMM2, but not use the background tracking result of GMM2, use AMF result at this time->when GMM2 is ready, use result of GMM2 rather than AMF

As the tracking method, we use frame-to-frame data association method. For the removed or abandoned object detection, we use the attribute that the different Gaussian mixture model change differently to find the still area.

simple results:
 http://youtu.be/W8EZw9au-AQ
 
 
 

