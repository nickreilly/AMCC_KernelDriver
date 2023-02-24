"""\
this file is basically a postscript program to aid in printing.
"""

__version__ =\
"""
    Version:
        $Id: psproc.py,v 1.1 2003/10/03 02:24:42 drew Exp $
    Revison:
        $Log: psproc.py,v $
        Revision 1.1  2003/10/03 02:24:42  drew
        initial version of postscript stuff. Still have not
        run it yet.

"""

psproc = """\
% ------------------------------ ps_proc begin  ----------------------
%  Define procedures
% --------------------------------
% inch - translate inches to coord
%        stack:  inch => coord
/inch {72 mul} def

% lineproc - Defines a path for a line between two points.
%       stack:  x1 y1 x2 y2 => ---
/lineproc
{
    0 begin
       /y2 exch def  %  y2
       /x2 exch def  %  x2
       /y1 exch def  %  y1
       /x1 exch def  %  x1
		 newpath
		 x1 y1 moveto
		 x2 y2 lineto
	end
} def
/lineproc load 0 4 dict put
%
% boxproc - Defines a path as a box.
%       stack:  x y wid hgt => ---
/boxproc
{  0 begin 
      newpath 
      /p1 exch def  % hgt
		/p2 exch def  % wid
		/p3 exch def  % y
		/p4 exch def  % x
		p4 p3 moveto    % x y moveto
		p2  0 rlineto   % wid 0 rlineto 
		0  p1 rlineto   % 0 hgt rlineto 
		p2 neg 0 rlineto   % -wid 0 rlineto 
	   closepath
	 end
} def
/boxproc load 0 4 dict put
%
% cboxproc - Defines a path as a box centered at x, y.
%       stack:  x y wid hgt => ---
/cboxproc
{
   0 begin
       /p1 exch def  % hgt
		 /p2 exch def  % wid
		 /p3 exch def  % y
		 /p4 exch def  % x
		 p4 p2 2 div sub  % x = x-(wid/2), stack: nx
		 p3 p1 2 div sub  % y = y-(wid/2), stack: nx, ny
		 p2 p1 boxproc  % use box to draw cbox
	end
} def
/cboxproc load 0 4 dict put
%
% dpointproc - Draws a point
%          stack: x, y, rad, err_hgt 
/dpointproc
{
   0 begin
      /err_hgt exch def   % err_hgt ( 1 std dev )
      /rad exch def       % rad - radius of dot & width of bars
      /y exch def   % y
      /x exch def   % x
      newpath
      x y rad 90 450 arc
      stroke
      % draw error bar
      0 err_hgt ne   
      {
         x y err_hgt add x y err_hgt sub lineproc
         stroke
         x rad sub y err_hgt add x rad add y err_hgt add lineproc
         stroke
         x rad sub y err_hgt sub x rad add y err_hgt sub lineproc
         stroke
      } if
   end  
} def
/dpointproc load 0 4 dict put
%
% ltextproc -  
%            stack: x, y, s
/ltextproc 
{
   0 begin
      /s exch def   % string to print
      /y exch def   % y
      /x exch def   % x
      x y moveto
      s show
   end
} def
/ltextproc load 0 3 dict put
%
% ctextproc - prints text center at x
%          stack: x, y, s
/ctextproc   
{
   0 begin
      /s exch def  % string to print
      /y exch def   % y
      /x exch def   % x
      x
      s stringwidth
      pop    
      2 div sub
      y moveto
      s show
   end
} def
/ctextproc load 0 3 dict put
%
% rtextproc - print text right justified
%          stack: x, y, s
/rtextproc
{
   0 begin
      /s exch def  % string to print
      /y exch def   % y
      /x exch def   % x
      s stringwidth
      pop neg x add
      y moveto
      s show
   end
} def
/rtextproc load 0 3 dict put
%
%  bitdumpproc -  processes a bitmap for display.
%         stack: width, height
/bitdumpproc
{
   0 begin
      % read arguments
      /height exch def
      /width exch def
      /picstr width string def % picstr holds one scan line

      % read and dump the image
      width height 8 [width 0 0 height neg 0 height]
      {  currentfile picstr readhexstring pop }
      image
   end
} def
/bitdumpproc load 0 3 dict put
%
%  colordumpproc -  processes a bitmap for display.
%         stack: width, height
%
/colordumpproc
{
   0 begin
      % read arguments
      /height exch def
      /width exch def
      /picstr width 3 mul string def % picstr holds one scan line
 
      % read and dump the image
      width height 8 [width 0 0 height neg 0 height]
      {  currentfile picstr readhexstring pop }
      false 3
      colorimage
   end
} def
/colordumpproc load 0 3 dict put
% ------------------------------ End of function definition  ----------------------
%
/Helvetica findfont 20 scalefont setfont
36 722 (University of Rochester Near Infrared Astronomy Lab) ltextproc
/Helvetica findfont 20 scalefont setfont
36 700 (DSP System) ltextproc
%
% ------------------------------ End of ps_proc   ----------------------
%"""
 
