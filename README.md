# CRISP_Summer_2019
Arduino code for the mobile manipulator in the CRISP Lab at Tufts University. Written by Victor Arsenescu under the supervision of Professor Messner, Professor Bedell, and Professor Panetta. 

Descriptions:

OSIRIS_sweep:
  OSIRIS for user session on 10/22/19 - IR speed control and serial input directional control with bi-directional homing    function.

virtualZen: 
  Positions the platform of the mobile manipulator based on dead reckoning. A button (absolute zero) is hit during the 
  initialization sequence, and from there on encoder counts for the z-axis DC motor are used.
  
-> In general, code is heavily commented to aid comprehension. 
  
  *Note*: Absolute zero button must be pre-configured to the range of the scissor lift.
  
  *Note*: viaremote.h is remote specific - new values must be mapped for new IR remotes. 
