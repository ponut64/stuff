SET /P this=FILE: 
bin2hex --i %this%.bin --o %this%.c --b 4
ECHO this %this%
PAUSE