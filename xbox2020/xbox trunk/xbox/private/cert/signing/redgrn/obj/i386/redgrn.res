L ���<�Q  	      .debug$S        E   �               @ B.rsrc$01        �   �   ~         @  @.rsrc$02         P  �              @  @    	     obj\i386\redgrn.res#    Microsoft CVTRES 5.00.2134.1               �  �   �   8  �                  P  �                  h  �               	  �                  	  �       �L              d           F I L E   �       �       d4   V S _ V E R S I O N _ I N F O     ���       0    0?   	                    �   S t r i n g F i l e I n f o   �   0 4 0 9 0 4 B 0   L   C o m p a n y N a m e     M i c r o s o f t   C o r p o r a t i o n   v '  F i l e D e s c r i p t i o n     X b o x   R e d - t o - G r e e n   t a p e   s i g n i n g   u t i l i t y     8   F i l e V e r s i o n     1 . 0 0 . 4 4 0 0 . 1   D   I n t e r n a l N a m e   x b o x g r e e n s i g n . e x e   t (  L e g a l C o p y r i g h t   C o p y r i g h t   ( C )   M i c r o s o f t   C o r p .   1 9 8 1 - 2 0 0 1   L   O r i g i n a l F i l e n a m e   x b o x g r e e n s i g n . e x e   L   P r o d u c t N a m e     M i c r o s o f t ( R )   X b o x ( T M )   <   P r o d u c t V e r s i o n   1 . 0 0 . 4 4 0 0 . 1   D    V a r F i l e I n f o     $    T r a n s l a t i o n     	�    <?xml version="1.0" encoding="UTF-8"?>
<!--
This is the XML Schema for DX2ML 1.0. (DVD-X2 Mastering Language)

This document is Copyright 2000-2001, Doug Carson and Associates, Inc. (DCA) All Rights Reserved
-->
<?xml-stylesheet href="xdr-schema.xsl" type="text/xsl"?>
<Schema name="dx2ml" xmlns="urn:schemas-microsoft-com:xml-data" xmlns:dt="urn:schemas-microsoft-com:datatypes">
        <AttributeType name="crc-32" dt:type="ui4" required="no">
                <description>This attribute specifies a CCITT CRC-32 checksum of a stream or disc region.</description>
        </AttributeType>
        <AttributeType name="length" dt:type="ui4" required="no">
                <description>This attribute specifies a length in sectors of an input-stream.</description>
        </AttributeType>
        <AttributeType name="length-used" dt:type="ui4" required="no">
                <description>This optional attribute specifies the amount of the referenced stream used in this reference.  If this attribute is not specified, all of the remaining unused portion of the referenced stream will be used.</description>
        </AttributeType>
        <AttributeType name="repeat" dt:type="ui4" required="no">
                <description>This optional attribute specifies the number of times to repeat the use of an input stream.</description>
        </AttributeType>
        <AttributeType name="id" dt:type="id" required="no">
                <description>This attribute uniquely identifies an element within a document. Its value is an XML identifier.</description>
        </AttributeType>
        <AttributeType name="stream-id" dt:type="idref" required="yes">
                <description>This required attribute specifies a reference to a stream defined in the &lt;input&gt; section.</description>
        </AttributeType>
        <AttributeType name="storage-mode" dt:type="enumeration" dt:values="DATA2048 DATA2054 INCOMPLETE2064 COMPLETE2064" required="no" default="DATA2048">
                <description>This attribute specifies size and data contained in the &lt;input-stream&gt; in relation to a normalized 2064 input stream. The &lt;input-stream&gt; file can be stored in 2048 byte, 2054 byte, or 2064 byte sectors.</description>
        </AttributeType>
        <AttributeType name="file-name" dt:type="string" required="yes">
                <description>This attribute specifies a text name of the input file on tape or network.</description>
        </AttributeType>
        <AttributeType name="title" dt:type="string" required="no">
                <description>This optional attribute specifies a text title that may be applied to this element.</description>
        </AttributeType>
        <AttributeType name="content" dt:type="string" required="no">
                <description>This optional attribute specifies the type of content in the referenced stream.</description>
        </AttributeType>
        <ElementType name="dx2ml" model="closed" content="eltOnly" order="seq">
                <description>The dx2ml element is the top level element of a DX2ML document.</description>
                <attribute type="id"/>
                <AttributeType name="xmlns" dt:type="string"/>
                <attribute type="xmlns"/>
                <element type="input" minOccurs="1" maxOccurs="1"/>
                <element type="disc" minOccurs="1" maxOccurs="1"/>
                <element type="mastering-job" minOccurs="1" maxOccurs="1"/>
        </ElementType>
        <ElementType name="input" model="closed" content="eltOnly" order="seq">
                <description>This element contains information about the input media required to create a disc.</description>
                <element type="media" minOccurs="1" maxOccurs="1"/>
                <element type="media" minOccurs="0" maxOccurs="1"/>
        </ElementType>
        <ElementType name="media" model="closed" content="eltOnly" order="seq">
                <description>This element defines an input media.</description>
                <AttributeType name="type" dt:type="enumeration" dt:values="TAPE NETWORK" default="TAPE" required="no">
                        <description>This attribute specifies whether the input media is "TAPE" or "NETWORK" (file) based. The default value of the type attribute is "TAPE".</description>
                </AttributeType>
                <AttributeType name="path" dt:type="uri" required="no">
                        <description>This optional attribute specifies the URI of a directory or folder containing media files in a network environment.</description>
                </AttributeType>
                <attribute type="id" required="yes"/>
                <attribute type="type"/>
                <attribute type="path"/>
                <attribute type="title"/>
                <attribute type="crc-32"/>
                <element type="leadin-stream" minOccurs="1" maxOccurs="1"/>
                <element type="xbox-leadout-stream" minOccurs="1" maxOccurs="1"/>
                <element type="input-stream" minOccurs="1" maxOccurs="*"/>
                <element type="crc-stream" minOccurs="0" maxOccurs="1"/>
        </ElementType>
        <ElementType name="leadin-stream" model="closed" content="empty">
                <description>This element describes an input file containing the DVD lead-in control data.
    </description>
                <attribute type="id" required="yes"/>
                <attribute type="length" default="16"/>
                <attribute type="storage-mode"/>
                <attribute type="file-name"/>
                <attribute type="crc-32"/>
        </ElementType>
        <ElementType name="xbox-leadout-stream" model="closed" content="empty">
                <description>This element describes an input file containing the DVD-X2 lead-out control data.</description>
                <attribute type="id" required="yes"/>
                <attribute type="length" default="32"/>
                <attribute type="storage-mode"/>
                <attribute type="file-name"/>
                <attribute type="crc-32"/>
        </ElementType>
        <ElementType name="input-stream" model="closed" content="empty">
                <description>This element describes an input file.</description>
                <attribute type="id" required="yes"/>
                <attribute type="length" required="yes"/>
                <attribute type="storage-mode"/>
                <attribute type="file-name"/>
                <attribute type="crc-32"/>
        </ElementType>
        <ElementType name="crc-stream" model="closed" content="empty">
                <description>This element describes a crc value file.</description>
                <attribute type="id" required="yes"/>
                <attribute type="length" default="16"/>
                <attribute type="storage-mode"/>
                <attribute type="file-name" default="crc.xml"/>
        </ElementType>
        <ElementType name="content-parameter" model="closed" content="empty">
                <description>This element can be used to define parameters for special content processing. Each &lt;content-parameter&gt; element specifies a single parameter/value pair. The list of parameters is open-ended.</description>
                <AttributeType name="name" dt:type="nmtoken" required="yes">
                        <description>This required attribute identifies the parameter defined in this element.</description>
                </AttributeType>
                <AttributeType name="value" dt:type="string" required="yes">
                        <description>This required attribute specifies the value of the parameter defined in this element.</description>
                </AttributeType>
                <attribute type="name"/>
                <attribute type="value"/>
        </ElementType>
        <ElementType name="leadin-stream-ref" model="closed" content="eltOnly">
                <description>This element contains a reference to an &lt;leadin-stream&gt; defined in the &lt;input&gt; section.</description>
                <attribute type="stream-id"/>
                <attribute type="length-used" default="16"/>
                <attribute type="repeat" default="192"/>
                <attribute type="content" default=""/>
                <element type="content-parameter" minOccurs="0" maxOccurs="*"/>
        </ElementType>
        <ElementType name="xbox-leadout-stream-ref" model="closed" content="eltOnly">
                <description>This element contains a reference to an &lt;xbox-leadout-stream&gt; defined in the &lt;input&gt; section.</description>
                <attribute type="stream-id"/>
                <attribute type="length-used" default="32"/>
                <attribute type="repeat" default="80"/>
                <attribute type="content" default=""/>
                <element type="content-parameter" minOccurs="0" maxOccurs="*"/>
        </ElementType>
        <ElementType name="input-stream-ref" model="closed" content="eltOnly">
                <description>This element contains a reference to an &lt;input-stream&gt; defined in the &lt;input&gt; section.</description>
                <attribute type="stream-id"/>
                <attribute type="length-used"/>
                <attribute type="content" default=""/>
                <element type="content-parameter" minOccurs="0" maxOccurs="*"/>
        </ElementType>
        <ElementType name="reference-code-stream" model="closed" content="eltOnly">
                <description>This element contains generated data necessary to create a reference code zone as defined in ECMA-267 26.2.</description>
                <attribute type="length-used" default="16"/>
                <attribute type="repeat" default="2"/>
                <attribute type="content" default=""/>
                <element type="content-parameter" minOccurs="0" maxOccurs="*"/>
        </ElementType>
        <ElementType name="zero-stream" model="closed" content="eltOnly">
                <description>This element defines a generated stream that contains data consisting of zeros.</description>
                <attribute type="length-used" required="yes"/>
                <attribute type="content" default=""/>
                <element type="content-parameter" minOccurs="0" maxOccurs="*"/>
        </ElementType>
        <ElementType name="disc" model="closed" content="eltOnly" order="seq">
                <description>This element contains information about the layers required to create a disc.</description>
                <AttributeType name="abstract" dt:type="string" required="no">
                        <description>This optional attribute specifies a brief description of the content contained in the element.</description>
                </AttributeType>
                <AttributeType name="author" dt:type="string" required="no">
                        <description>This optional attribute specifies the name of the author of the content contained in the element.</description>
                </AttributeType>
                <AttributeType name="copyright" dt:type="string" required="no">
                        <description>This optional attribute specifies the copyright notice of the content contained in the element.</description>
                </AttributeType>
                <AttributeType name="data-crc" dt:type="ui4" required="no">
                        <description>This optional attribute specifies a CCITT CRC-32 checksum of all user data contained on the standard DVD portion of the disc.</description>
                </AttributeType>
                <AttributeType name="xdata-crc" dt:type="ui4" required="no">
                        <description>This optional attribute specifies a CCITT CRC-32 checksum of all user data (exclusive of DVD-X2-PLACEHOLDER content) contained on the Xbox portion of the disc.</description>
                </AttributeType>
                <AttributeType name="xcheck-crc" dt:type="ui4" required="no">
                        <description>This optional attribute specifies a CCITT CRC-32 checksum of all user data (exclusive of DVD-X2-PLACEHOLDER content and XBOX- EXECUTABLE content modified by signing) contained on the Xbox portion of the disc.</description>
                </AttributeType>
                <AttributeType name="type" dt:type="enumeration" dt:values="DVD-X2" required="yes">
                        <description>This required attribute specifies the type of disc.  This value must be set to "DVD-X2".</description>
                </AttributeType>
                <AttributeType name="size" dt:type="int" required="no">
                        <description>This attribute specifies the physical diameter of the disc in mm.  The default value is "120".</description>
                </AttributeType>
                <attribute type="id"/>
                <attribute type="title"/>
                <attribute type="abstract"/>
                <attribute type="author"/>
                <attribute type="copyright"/>
                <attribute type="data-crc"/>
                <attribute type="xdata-crc"/>
                <attribute type="xcheck-crc"/>
                <attribute type="type"/>
                <attribute type="size" default="120"/>
                <element type="side" minOccurs="1" maxOccurs="1"/>
                <element type="side" minOccurs="0" maxOccurs="1"/>
        </ElementType>
        <ElementType name="side" model="closed" content="eltOnly" order="seq">
                <description>This contains information about a single side of the disc.  The side elements shall be defined in the order of side 1 then side 2.</description>
                <attribute type="id"/>
                <element type="layer" minOccurs="1" maxOccurs="1"/>
                <element type="layer" minOccurs="0" maxOccurs="1"/>
        </ElementType>
        <ElementType name="layer" model="closed" content="eltOnly" order="seq">
                <description>The "layer" element contains information about a single layer of the disc.  The layer elements shall be defined in the order of layer 0 then layer 1.</description>
                <AttributeType name="direction" dt:type="enumeration" dt:values="INNER-TO-OUTER OUTER-TO-INNER" default="INNER-TO-OUTER" required="no">
                        <description>This optional attribute specifies the direction of translation of the layer.  Valid values are "INNER-TO-OUTER" and "OUTER-TO-INNER".  The default value is "INNER-TO-OUTER".</description>
                </AttributeType>
                <attribute type="id" required="yes"/>
                <attribute type="direction"/>
                <element type="main-band" minOccurs="1" maxOccurs="1"/>
        </ElementType>
        <ElementType name="main-band" model="closed" content="eltOnly" order="seq">
                <description>This element contains information about the Information Area of a single layer of the disc.</description>
                <AttributeType name="start-address" dt:type="bin.hex" required="yes">
                        <description>This required attribute specifies the starting Physical Sector Number of the band. If the layer has "INNER-TO-OUTER" translation, this should be the address of the inner most physical sector of the band.  If the layer has "OUTER-TO-INNER" translation, this should be the address of the outer most physical sector of the band.</description>
                </AttributeType>
                <attribute type="start-address"/>
                <element type="region" minOccurs="1" maxOccurs="*"/>
        </ElementType>
        <ElementType name="region" model="closed" content="eltOnly" order="seq">
                <description>This delineates a logical region of the disc.</description>
                <AttributeType name="type" dt:type="enumeration" dt:values="LEAD-IN DATA MIDDLE LEAD-OUT X-LEAD-IN X-DATA X-MIDDLE X-LEAD-OUT" required="yes">
                        <description>This required attribute specifies the type of disc region. Valid values are "LEAD-IN: (DVD Standard Lead-In area), "DATA" (DVD Standard Data area), "MIDDLE" (DVD Standard Middle area), "LEAD-OUT" (DVD Standard Lead-Out area), "X-LEAD---IN" (XBox Lead-In area), "X-DATA" (XBox Data area), "X-MIDDLE" (XBox Middle area), "X-LEAD-OUT" (XBox Lead-Out area).</description>
                </AttributeType>
                <attribute type="type"/>
                <attribute type="crc-32"/>
                <group minOccurs="1" maxOccurs="*" order="one">
                        <element type="zero-stream" minOccurs="1" maxOccurs="1"/>
                        <element type="input-stream-ref" minOccurs="1" maxOccurs="1"/>
                        <element type="leadin-stream-ref" minOccurs="1" maxOccurs="1"/>
                        <element type="xbox-leadout-stream-ref" minOccurs="1" maxOccurs="1"/>
                        <element type="reference-code-stream" minOccurs="1" maxOccurs="1"/>
                </group>
        </ElementType>
        <ElementType name="mastering-job" model="open" content="eltOnly" order="seq">
                <description>This element contains information about the specific layer to be processed.</description>
                <AttributeType name="media-ref" dt:type="idref" required="yes">
                        <description>This required attribute specifies which input media is to be used.</description>
                </AttributeType>
                <AttributeType name="side-ref" dt:type="idref" required="no">
                        <description>This optional attribute specifies which side is to be processed.</description>
                </AttributeType>
                <AttributeType name="layer-ref" dt:type="idref" required="yes">
                        <description>This required attribute specifies which layer is to be processed.</description>
                </AttributeType>
                <AttributeType name="master-id" dt:type="string" required="no">
                        <description>This optional attribute is used to convey a master serial id to the processing system.</description>
                </AttributeType>
                <attribute type="media-ref"/>
                <attribute type="side-ref"/>
                <attribute type="layer-ref"/>
                <attribute type="master-id"/>
                <element type="meta" minOccurs="0" maxOccurs="*"/>
        </ElementType>
        <ElementType name="meta" model="closed" content="empty">
                <description>This element can be used to define properties of a job and assign values to those properties. Each &lt;meta&gt; element specifies a single property/value pair. The list of properties is open-ended.</description>
                <AttributeType name="name" dt:type="nmtoken" required="yes">
                        <description>This required attribute identifies the property defined in this element.</description>
                </AttributeType>
                <AttributeType name="value" dt:type="string" required="yes">
                        <description>This required attribute specifies the value of the property defined in this element.</description>
                </AttributeType>
                <attribute type="name"/>
                <attribute type="value"/>
        </ElementType>
</Schema>
  @comp.idV ��   .debug$S       E                 .rsrc$01       �                .rsrc$02        P                $R000000        $R000368h          