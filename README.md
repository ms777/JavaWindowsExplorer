<HTML>
<HEAD>
	<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="text/html; charset=windows-1252">
	<TITLE></TITLE>
	<META NAME="GENERATOR" CONTENT="OpenOffice.org 3.4.1  (Win32)">
	<META NAME="CREATED" CONTENT="20140215;12120840">
	<META NAME="CHANGED" CONTENT="20140216;461900">
	<STYLE TYPE="text/css">
	</STYLE>
</HEAD>
<BODY LANG="de-DE" DIR="LTR">
<H1>Java Windows Explorer</H1>
<P STYLE="margin-bottom: 0cm">Windows Explorer has many capabilities,
which are difficult or cumbersome to implement in Java. This software
allows to include a Windows Explorer into Java Swing applications.
You can do everything from within Java, what you can do normally from
Windows. 
</P>
<P STYLE="margin-bottom: 0cm">The demo application shows, how to
access windows search capabilites from Java.</P>
<P STYLE="margin-bottom: 0cm"><BR>
</P>
<P STYLE="margin-bottom: 0cm">Please note that this software does not
allow to include an Internet Explorer window into Java. There exist
already enough tools which allow for this.</P>
<H2 CLASS="western">Features:</H2>
<UL>
	<LI><P STYLE="margin-bottom: 0cm">Embedding a Windows Explorer
	window into your Java Swing application</P>
	<LI><P STYLE="margin-bottom: 0cm">Navigation both by mouse and by
	code</P>
	<LI><P STYLE="margin-bottom: 0cm">You can add custom options to the
	right click menu. These options may depend on the currently focused
	item</P>
	<LI><P STYLE="margin-bottom: 0cm">You can use Windows search by
	code, with very detailed control over search scope. I found that
	Windows search is much better than its reputation &hellip;</P>
</UL>
<P STYLE="margin-bottom: 0cm">For a simple demo:</P>
<UL>
	<LI><P STYLE="margin-bottom: 0cm">download the suitable jar in
	ExplorerCancas/jar. If you do not know if your default Java is 32bit
	or 64bit download both.</P>
	<LI><P STYLE="margin-bottom: 0cm">doubleclick  the jar</P>
</UL>
<H2 CLASS="western">Requirements for running:</H2>
<UL>
	<LI><P STYLE="margin-bottom: 0cm">This software by its nature runs
	only under Windows, not Unix or other systems.</P>
	<LI><P STYLE="margin-bottom: 0cm">Windows7 is minimum, it is only
	tested under Windows7.</P>
	<LI><P STYLE="margin-bottom: 0cm">Java 7, both 32 and 64 bit
	operation is suported</P>
	<LI><P STYLE="margin-bottom: 0cm">You just need to download the
	appropriate 32 bit or 64 bit jar and include it into your Java
	project</P>
	<LI><P STYLE="margin-bottom: 0cm">The jar extracts the dll into a
	temporary path on your computer when loaded for the first time. On
	my system this is C:\Users\myname\AppData\Local\Temp\ExplorerCanvas.
	If you install a newer jar, you have to empty or delete this folder.</P>
</UL>
<H2 CLASS="western">Requirements for compiling the C++ source:</H2>
<UL>
	<LI><P STYLE="margin-bottom: 0cm">MinGW 64 bit compiler suite, also
	supports 32 bit builds. You can get it from here
	<A HREF="http://tdm-gcc.tdragon.net/">http://tdm-gcc.tdragon.net/</A>.
	4.8.1 version is used</P>
	<LI><P STYLE="margin-bottom: 0cm">This is to the best of my
	knowledge the only MinGW based compiler suite with enough windows
	header files to support the build</P>
	<LI><P STYLE="margin-bottom: 0cm">Some functions of shell32 are
	included in the MinGW header files, but not in the MinGW library.
	This is why you have to include libshell32_ms777.a. Generation of
	the libshell32_ms777.a is described in the batch files in shell32 
	</P>
</UL>
<P STYLE="margin-bottom: 0cm"><BR>
</P>
<P STYLE="margin-bottom: 0cm">This is work in process and my first
project in C++. So I am quite sure that there are many memory leaks &hellip;</P>
<P STYLE="margin-bottom: 0cm"><BR>
</P>
<P ALIGN=LEFT STYLE="margin-bottom: 0cm"><FONT COLOR="#000000"><FONT FACE="Courier New, monospace"><FONT SIZE=2>Copyright
2014 </FONT></FONT></FONT><FONT COLOR="#000000"><SPAN STYLE="text-decoration: none"><FONT FACE="Courier New, monospace"><FONT SIZE=2>Martin
Schell</FONT></FONT></SPAN></FONT></P>
<P ALIGN=LEFT STYLE="margin-bottom: 0cm"><FONT COLOR="#000000"><FONT FACE="Courier New, monospace"><FONT SIZE=2>Licensed
under the <SPAN STYLE="text-decoration: none">Apache</SPAN> License,
Version 2.0 (the &quot;License&quot;); you may not use this file
except in compliance with the License. You may obtain a copy of the
License at</FONT></FONT></FONT></P>
<P ALIGN=LEFT STYLE="margin-bottom: 0cm"><FONT COLOR="#000000"><FONT FACE="Courier New, monospace"><FONT SIZE=2>http://www.apache.org/licenses/LICENSE-2.0</FONT></FONT></FONT></P>
<P ALIGN=LEFT STYLE="margin-bottom: 0cm"><FONT COLOR="#000000"><FONT FACE="Courier New, monospace"><FONT SIZE=2>Unless
required by applicable law or agreed to in writing, software
distributed under the License is distributed on an &quot;AS IS&quot;
BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
or implied. See the License for the specific language governing
permissions and limitations under the License.</FONT></FONT></FONT></P>
<H3 CLASS="western" STYLE="margin-top: 0cm; margin-bottom: 0cm"><BR>
</H3>
</BODY>
</HTML>