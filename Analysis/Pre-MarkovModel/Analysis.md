##Interior Generation Analysis##

#Overlapping model#
```xml
-----------------------------------------------------------------
Problem Details:
Periodic Output: true
Periodic Input: true
width : 12
height : 12

Notes:
- Not completely "sensible" outputs
- <NotSensible>

Run:
Run_15-10-2019_15-38-24

File:
Problem_48_OfficeInterior1_0


<------------------------------------->
Problem Details:
<overlapping name="OfficeInterior1"	N="5" width="5" height="7" periodic="false"/>

Notes:
- <Sensible>

Run:
Run_15-10-2019_15-38-24

File:
Problem_119_OfficeInterior1_1

-----------------------------------------------------------------

Problem Details:
<overlapping name="OfficeInterior1"	N="5" width="5" height="7" periodic="false"/>

Notes:
- Created table with no entry point
- <NotSensible>

Run:
Run_15-10-2019_15-38-24

File:
Problem_119_OfficeInterior1_0

<------------------------------------->
Problem Details:
<overlapping name="OfficeInterior1"	N="5" width="5" height="7" periodicInput="true" periodic="false"/>

Notes:
- <Sensible>

Run:
Run_15-10-2019_15-38-24

File:
Problem_118_OfficeInterior1_1

-----------------------------------------------------------------

Problem Details:
<overlapping name="OfficeInterior1"	N="5" width="5" height="7" periodicInput="true" periodic="false"/>

Notes:
- <NotSensible>

Run:
Run_15-10-2019_15-38-24

File:
Problem_118_OfficeInterior1_0

<------------------------------------->

Problem Details:
<overlapping name="OfficeInterior1"	N="3" width="5" height="7" periodicInput="false" periodic="false" symmetry="7"/>

Notes:
- <Sensible>

Run:
Run_15-10-2019_15-38-24

File:
Problem_117_OfficeInterior1_0

-----------------------------------------------------------------

Problem Details:
<overlapping name="OfficeInterior1"	N="3" width="5" height="7" periodicInput="false" periodic="false" symmetry="7"/>

Notes:
- <NotSensible>

Run:
Run_15-10-2019_15-38-24

File:
Problem_117_OfficeInterior1_1

<------------------------------------->

Problem Details:
<overlapping name="OfficeInterior1"	N="3" width="5" height="7" periodicInput="false" periodic="false" symmetry="6"/>

Notes:
- <Sensible>

Run:
Run_15-10-2019_15-38-24

File:
Problem_116_OfficeInterior1_0

-----------------------------------------------------------------

Problem Details:
<overlapping name="OfficeInterior1"	N="3" width="5" height="7" periodicInput="false" periodic="false" symmetry="6"/>

Notes:
- <NotSensible>

Run:
Run_15-10-2019_15-38-24

File:
Problem_116_OfficeInterior1_1
```

#Viable for Overlapping Model#

```xml
<SomewhatDesiredResults>

<overlapping name="OfficeInterior1"	N="5" width="5" height="7" periodic="false"/> 	<TryNoWrapInput>
<overlapping name="OfficeInterior1"	N="5" width="5" height="7" periodicInput="true" periodic="false"/>  <TryNoWrapInput>
<overlapping name="OfficeInterior1"	N="3" width="5" height="7" periodicInput="false" periodic="false" symmetry="7"/>
<overlapping name="OfficeInterior1"	N="3" width="5" height="7" periodicInput="false" periodic="false" symmetry="6"/>
<overlapping name="OfficeInterior1"	N="3" width="5" height="7" periodicInput="false" periodic="false" symmetry="5"/>
<overlapping name="OfficeInterior1"	N="3" width="5" height="7" periodicInput="false" periodic="false" symmetry="4"/>
<overlapping name="OfficeInterior1"	N="3" width="7" height="7" periodicInput="false" periodic="false" symmetry="3"/>
<overlapping name="OfficeInterior1"	N="3" width="7" height="7" periodicInput="false" periodic="false" symmetry="2"/>

<NoChangeSet>

<overlapping name="OfficeInterior1"	N="3" width="5" height="7" periodicInput="false" periodic="false" symmetry="2"/>
<overlapping name="OfficeInterior1"	N="3" width="5" height="7" periodicInput="false" periodic="false"/>
<overlapping name="OfficeInterior1"	N="3" width="5" height="7" periodic="false"/>
<overlapping name="OfficeInterior1"	N="5" width="7" height="7" periodicInput="true" periodic="false"/>
<overlapping name="OfficeInterior1"	N="3" width="7" height="7" periodicInput="false" periodic="false" symmetry="7"/>
<overlapping name="OfficeInterior1"	N="3" width="7" height="7" periodicInput="false" periodic="false" symmetry="6"/>
<overlapping name="OfficeInterior1"	N="3" width="7" height="7" periodicInput="false" periodic="false"/>
<overlapping name="OfficeInterior1"	N="3" width="7" height="7" periodic="false"/>
<overlapping name="OfficeInterior1" N="3" width="7" height="7" periodic="True"/>


```

Notes:
