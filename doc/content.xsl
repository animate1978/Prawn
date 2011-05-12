<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
 

<xsl:template match="/">
	<html>

	<body bgcolor="#333333" link="#435571" vlink="#a6c4f7">
	<font color="#edeceb"> 
	<a name="stcontent"></a>
	<h2> Index </h2>
	<code>Number of blocks:<xsl:value-of select="count(//shrimp)"/></code>
 	<hr width="100%"> </hr>



	<table border="0">
	<xsl:for-each select="block/shrimp">
	<xsl:sort select="@name"/>
		<table>

		<xsl:call-template name="text-loop">
		  <xsl:with-param name="max-times" select="27"/>
		  <xsl:with-param name="node" select="@name"/>
		</xsl:call-template>
	  	</table>
	</xsl:for-each>
	</table>
	</font>
	</body>
	</html>
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
