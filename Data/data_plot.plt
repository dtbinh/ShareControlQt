f0 = 'scs\expTTT-state-0.txt'
f1 = 'scs\expTTT-state-1.txt'
f2 = 'scs\expTTT-state-2.txt'
f3 = 'scs\expTTT-state-3.txt'

g0 = 'scs\expNon-state-0.txt'
g1 = 'scs\expNon-state-1.txt'
g2 = 'scs\expNon-state-2.txt'
g3 = 'scs\expNon-state-3.txt'

R = 1
r = 0.3
r2 = 1.2 * r
r3 = r2 * 1.2

#set term wxt size 500,400
set term pngcairo size 700,500
set output 'res.png'
set size ratio -1
set xrange [-(R+r+r3):(R+r+r3)]
set yrange [-(R+r+r3):(R+r+r3)]

set object 1 rect from -(R+r), (R+r) to (R+r), -(R+r) fs border lc black
set object 2 rect from -(R-r), (R-r) to (R-r), -(R-r) fs border lc black
set object 3 circle at -(R+r), 0 size r2 fs border lc black
set object 4 circle at (-R/2.0), R+r size r2 fs border lc black
set object 5 circle at (R/2.0), R+r size r2 fs border lc black
set object 6 circle at R, 0 size r2/2 fs border lc black
set object 7 circle at (R/2.0), -(R+r) size r2 fs border lc black
set object 8 circle at (-R/2.0), -(R-r) size r2 fs border lc black

t1 = 100
set key b rmargin
plot f0 u 'x':'y' every ::1::t1 w l lw 1.5 title 'robot-1', \
	 f1 u 'x':'y' every ::1::t1 w l lw 1.5 title 'robot-2', \
	 f2 u 'x':'y' every ::1::t1 w l lw 1.5 title 'robot-3', \
	 f3 u 'x':'y' every ::1::t1 w l lw 1.5 title 'robot-4'
