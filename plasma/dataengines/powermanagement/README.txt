TODO:
======
- some machines have more than one battery
- find a way to compute remaining minutes
- Sleepstates don't match what solidshell reports
- ac plug state does not get updated
- this engine probably shares some functionality with
  the solidengine, have a look at that and evaluate

Notes
======
Check out into workspace/plasma/engines/

There's also a battery applet which uses this engine, find it in
the

Patch plasma/engines/CMakeLists.txt like this to enable building 
of the battery engine:

-- sebas

Index: CMakeLists.txt
===================================================================
--- CMakeLists.txt      (revision 680923)
+++ CMakeLists.txt      (working copy)
@@ -1,4 +1,5 @@
 add_subdirectory("time")
+add_subdirectory("powermanagement")
 add_subdirectory("soliddevice")
 add_subdirectory("systemmonitor")