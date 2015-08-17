An intelligent real-time bandwidth monitoring tool with a nice web interface.

It uses libpcap (through python) to scan the packets passing through a specific interface, the web interface is notified via JSON and it updates the HTML5 canvas graphic; besides the web interface it uses Neural Networks (Self-Organizing Maps) to identify congestion and abuse, if either is found, an email is sent to the Sysamin.

Here is an example screenshot of the web interface:

![![](http://rtbm.googlecode.com/svn-history/r10/wiki/images/rtbm_small.png)](http://rtbm.googlecode.com/svn-history/r10/wiki/images/rtbm.png)

Makes it very easy to find patterns and possible offenders, :).