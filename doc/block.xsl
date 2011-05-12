<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
	<html>
	<!--
    Copyright 2009, Romain Behar &#60;romainbehar&#64;users.sourceforge.net&#62;

    This file is part of Shrimp 2.

    Shrimp 2 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Shrimp 2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Shrimp 2.  If not, see &#60;http://www.gnu.org/licenses/&#62;.
	-->
	
	<body bgcolor="#333333">
	<font color="#dadee3">
	<title>Shrimp 2.0 : Standard Blocks</title>


	<a name="stblocks"></a><br/>
	<img src="splash.png" alt="Shrimp 2.0"></img>
	<h1>Shrimp 2.0 : Standard Blocks</h1>
	<xsl:for-each select="block/shrimp">
	<xsl:sort select="@name"/>
		  <xsl:variable name="bc" select="@name"/>
		  <a  name="{$bc}">
			<table width="100%" border="1" bgcolor="#435571">
			<td><font color="#ffffff"><h2><ul><xsl:value-of select="@name"/></ul></h2></font></td>
			</table>
		  </a>
		    <hr width="100%"> </hr>
		    <table border="1">
		      <tr>
			<th bgcolor="#435571">Type</th>
			<td><xsl:value-of select="@type"/></td>
		      </tr>
		      
		      <tr>
			<th bgcolor="#435571">Description</th>
			<td><dfn><xsl:value-of select="@description"/></dfn></td>
		      </tr>
		      <tr>
			<th bgcolor="#435571">Usage</th>
			<td><dfn><xsl:value-of select="usage/text()"/></dfn></td>
		      </tr>
		      <tr>
			<th bgcolor="#435571">Link</th>
			<td><dfn> <a href="{usage/rsl_link}"><xsl:value-of select="usage/rsl_link"/></a></dfn></td>
		      </tr>
		    </table>
		  <h3>Input</h3>
		  <table border="1">
		      <tr bgcolor="#435571">
			<th>Name</th>
			<th>Description</th>
			<th>Type</th>
			<th>Storage</th>
			<th>Default</th>
		      </tr>
	
		  	<xsl:for-each select="input">
			<tr>
				<td><xsl:value-of select="@name"/></td>
				<td><xsl:value-of select="@description"/></td>
				<td><xsl:value-of select="@type"/></td>
				<td><xsl:value-of select="@storage"/></td>
				<td><xsl:value-of select="@default"/></td>
			</tr>
		
			</xsl:for-each>
		  </table>
		  <h3>Output</h3>
		<table border="1">
		      <tr bgcolor="#435571">
			<th>Name</th>
			<th>Description</th>
			<th>Type</th>
		      </tr>
	
		  	<xsl:for-each select="output">
			<tr>
				<td><xsl:value-of select="@name"/></td>
				<td><xsl:value-of select="@description"/></td>
				<td><xsl:value-of select="@type"/></td>
			</tr>
		
			</xsl:for-each>
		  </table>
		  <h3>RSL Code(Xml format) </h3>
		  <td>Include :</td><code>
		  <b><xsl:value-of select="rsl_include"/></b></code>
		  <code>
		  <xsl:call-template name="break">
			<xsl:param name="code" select="shrimp/rsl_code"/>
		  </xsl:call-template>
		  </code>
		  <br/>
		  		
	</xsl:for-each>
	</font>
	</body>
	</html>
</xsl:template>
<xsl:template name="break">
	<xsl:param name="code" select="rsl_code"/>
	<xsl:choose>
		<xsl:when test="contains($code, '&#xa;')">
		      <xsl:value-of select="substring-before($code, '&#xa;')"/>
		      <br/>
		      <xsl:call-template name="break">
			  <xsl:with-param name="code" select="substring-after($code,'&#xa;')"/>
		      </xsl:call-template>
		</xsl:when>
		<xsl:when test="contains($code,';')">
      			<xsl:value-of select="substring-before($code, ';')"/>
      			<xsl:text>; </xsl:text> <br/>
			<xsl:call-template name="break">
      		        	<xsl:with-param name="code" select="substring-after($code,';')"/>
			</xsl:call-template>
   		</xsl:when>
		
	</xsl:choose>
</xsl:template>	

<xsl:template name="text-loop">
  <xsl:param name="max-times" select="1"/>
  <xsl:param name="node" select="1"/>
  <xsl:param name="i" select="1"/>

  <xsl:if test="$i &lt; $max-times">  
	<xsl:variable name="lcletters">abcdefghijklmnopqrstuvwxyz</xsl:variable>
	<xsl:variable name="ucletters">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>
    		<xsl:if test="substring($lcletters,$i,1) = substring($node,1,1) or substring($ucletters,$i,1) = substring($node,1,1) ">
			
			<tr>
			<xsl:variable name="bc" select="@name"/>
			<th><a href="{concat('index.xml#',$bc)}" target ="blocks" ><xsl:value-of select="$node"/></a></th>
			
			</tr>
  		</xsl:if>
    <xsl:call-template name="text-loop">
      <xsl:with-param name="max-times" select="$max-times"/>
      <xsl:with-param name="node" select="$node"/>
      <xsl:with-param name="i" select="$i + 1"/>
    </xsl:call-template>
  </xsl:if>
</xsl:template>
</xsl:stylesheet>


