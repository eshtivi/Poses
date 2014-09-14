Poses
=====

This code was developed by Elad Shtivi, under the supervision of Tal Hassner, at the Academic College of Tel-Aviv Yaffo. 

It was designed using the code from the calib and renderer project. 

If you use this code in your own project, please add references to either (or both of the papers):

T. Hassner, Viewing Real-World Faces in 3D, International Conference on Computer Vision (ICCV), Sydney, Austraila, Dec. 2013 (project, PDF)

T. Hassner, L. Assif, and L. Wolf, When Standard RANSAC is Not Enough: Cross-Media Visual Matching with Hypothesis Relevancy, Machine Vision and Applications (MVAP), Volume 25, Issue 4, Page 971-983, 2014 (PDF)

Please see the project page for code and additional information:
http://www.openu.ac.il/home/hassner/projects/poses/

=====

Quick Start: (Run on Windows without building)
------------

1. Download WinRelease folder's content:
* Poses.exe
* config.xml
* Dependencies\opencv246x86bin.zip
* Dependencies\OSG301x86bin.zip

2. Extract OpenCV from opencv246x86bin.zip

3. Extract OpenSceneGraph from OSG301x86bin.zip

4. Get Intraface files:
Download from http://www.humansensing.cs.cmu.edu/intraface/
I personally used v1.1 - other version may not be compatible
Copy from the Release/Models folders the following files:
* IntraFaceDLL.dll
* DetectionModel-v1.5.bin
* TrackingModel-v1.10.bin
* haarcascade_frontalface_alt2.xml

5. Edit config.xml to use your 3D model (see structure below)

6. Run poses.exe

=====

Few words regarding 3D models:

the provided config.xml file is ready for animating a 3D model I've found online
the model is called "female" by BonyFace - it's a quick bone based facial rigging tool for autodesk 3ds max
you'll find it under their download section
http://www.scriptattack.com/maxscripts/bonyface/index_eng.html
note that it is a .max file which isn't supported by OSG
I converted it to FBX in order to use it
(OSG support FBX via plugin which I compiled and included as well in the zip file)

you can use different models for reference (xml's config/processing/model) and display (xml's config/models/model[..])
but you must set them in the same scale and coordinates to work properly with the reference model's positional data

=====

config.xml:
-----------

* config\processing\model:
reference model to be rendered in frontal view for facial feature detection
its facial features points 3d coordinates will be used as a reference
+ lighting - enable light
+ pi_elevation - can be used to turn model in case it's facing the wrong direction. "0.5" will turn it from facing up (nose pointing up, head's top points toward screen) to front

* config\preprocessing
can be used to load previously processing data and save some loading time
preprocessing data will be written to "ref.xml" file when running poses in processing mode
+ enabled - turned off by default

* config\models
will contain list of models to be used
+ max - the maximum number of models supported at the same time

* config\models\model[1..n]
+ lighting - enable light
+ pi_elevation - pi factor for rotation
+ hideinterior - should hide model backface (back culling)
+ center - should center model

* config\models\model[1..n]\anim[1..m]
(optional) define bone animation according to one of the facial feature detected point
+ target - bone's name
+ point - point's number
+ anchor (optional) - point's number
if specified anchor, bone's movement will be according to distance from anchor's location instead of the original point's location
+ weightx/y/z - affect of each axis. 1 is normal. 0 eliminates movement in that axis. can be negative to change direction.

* config\input
+ type - "webcam"/"file"
for webcam, specify webcam's index (default webcam is 0. if you have more than one webcam, they'll be indexed as 0,1,2...)
for file, specify video file path

* config\output\file
set path to write output into a file (not recommended for webcam input)
+ enabled - should write to file or not. if "Y", you'll get a popup to select codec

* config\output\screen
on screen output
+ enabled - should show rendered output on screen
+ input - should show input video (including detected face points)
+ renderer - should show OSG's output using OSG window
+ tunnel - should get OSG's data and show it via opencv window (had problems showing OSG's window on some computers, so this is a workaround. prefer OSG's window if possible)
+ resize - should resize tunnel window since it's not resizable as OSG. 1 by default

=====

Intraface facial points:
------------------------

0 - 9 : eye browses
10-13 : nose bone
14-18 : nostril
19-30 : eyes
31-42 : outer lips
43-48 : inner lips



     1   2   3                          6   7   8
 0               4                  5               9
 
                            10
                            
      20   21                             26   27
 19            22           11       25            28
      24   23                             30   29
	  
	                        12
	                        
							
							13
							
					14		        18
					    15  16  17
						
						

				  33		34        35
	    32		                               36
		
			 43             44            45
31                                                   37
             48             47            46
			 
		42	                                   38
		          41        40         39
				  
				  
