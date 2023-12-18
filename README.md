
<!-- HTML comment -->

<h1 style="font-family:Helvetica;", align=center>[HEX]POD</h1><br>

<p align=center font-size=10px > by eBender <br>
⚠️ This project is in development.<br>
  Watch this space.</p>
  
  <p align=center><b><a href="https://hackaday.io/project/177083-h6x-pod">Hackaday</a> <br>
<a href="https://www.instagram.com/zen.diy/">Instagram</a><br>
<br>
<!-- <a href="https://discord.gg/3JU6GMgVZk">Discord</a> <br> -->
<!-- <a href="https://www.reddit.com/r/hex_pod/">Reddit</a> <br> -->
<!-- <a href="https://www.patreon.com/eBender">Patreon</a> <br><br> -->
<!-- <b>PCB production generously sponsored by <a href="https://www.pcbway.com">PCBway</a> -->
  </b></p>

<p>

<h4 align=center>A portable multi-purpose, intelligent, environmental multi-sensor platform and programmable 'Digital Nose'.</h4>

Technology has advanced significantly, with sensors being ubiquitous  in our devices and gathering a wealth of data. One thing we had to learn from Covid, there is a critical aspect of our quality of life that remains relatively neglected in the public sector:  <br>

The air we breathe. <br>

My mission is to empower people by raising awareness about airborne hazards, from aiding those with chronic respiratory conditions to detecting spoiled milk. While the concept of a 'Digital Nose' has been explored before, [HEX]POD is positioned as a user-friendly solution that seamlessly integrates cutting-edge technology into a compact and portable platform.

The [HEX]POD is a private development platform with focus on being a 'Digital Nose' with relatively cheap components, using Sensor fusion and Neural Networks to discern different chemicals and smells in the air.
</p>
 <br><br> 
 <hr style='height:1px'></hr>
<p align=center></b>
<h3>DEV KIT version</h3>
  The [HEX]POD dev Kit is a dense little cube. For now, the MVP version houses 4 Air quality sensors, 2 screens, a battery, 
  a blower fan and 16 I/O pins to hook up other modules and things and much more. It is a versatile 
  platform to play with Sensor Fusion, ML, NN and any other externally connected modules.
</p>

<hr style='height:1px'></hr>

![SIDE 2](https://github.com/EmanuelBender/HEX_POD/blob/main/images/Overview/SIDE%202.png)

![SIDE 1](https://github.com/EmanuelBender/HEX_POD/blob/main/images/Overview/SIDE%201.png)
<br>
<hr style='height:1px'></hr>

<!-- ![Cam7_2](https://github.com/EmanuelBender/HEX_POD/blob/main/images/Renders/Cam7_2.png) -->

![Cam11](https://github.com/EmanuelBender/HEX_POD/blob/main/images/Renders/Cam11.jpeg)

![Cam5](https://github.com/EmanuelBender/HEX_POD/blob/main/images/Renders/Cam5.jpeg)
<br>

<hr style='height:1px'></hr>
<p><h2>Dev Kit version [wip]</h2></p>
<table>
    <thead>
    <tr>
        <th>DETAILS</th>
        <th>Function</th>
        <th>Module</th>
    </tr>
    </thead>
    <tbody>
    <tr>
        <td><b>SENSORS</b></td>
        <td>Air Quality<br>Smells & Gases<br>Light Meter<br>Motion Sensor<br>Temp Sensors</td>
        <td>SCD41, SGP41 (<a href="https://www.sensirion.com/en/environmental-sensors/carbon-dioxide-sensors/carbon-dioxide-sensor-scd4x/" target="_blank">CO2</a>,&nbsp;&nbsp;<a href="https://www.sensirion.com/en/environmental-sensors/gas-sensors/sgp41/" target="_blank">VOC, NOx</a>)<br>BME688 (x2) (Gas, T, H, P + AI Gases)<br>LTR-308 (Ambient Light)<br>LIS3DH (x2) (MPU, 6-axis)<br>DS18B20 (x5)</td>
    </tr>
    <tr>
        <td><b>POWER</b></td>
        <td>15W Charging<br>Battery Life: tba</td>
        <td>2000mAh, 7.4Wh (<a href="https://www.aliexpress.com/item/1005002919536938.html" target="_blank">103450 Battery</a>)<br>tba</td>
    </tr>
    <tr>
        <td><b>SCREENS</b></td>
        <td>
            <a href="https://www.aliexpress.us/item/3256803567938962.html?spm=a2g0o.productlist.0.0.21743a4elfVKsE&algo_pvid=50a69a68-34bc-4972-be26-90207f61f1dd&algo_exp_id=50a69a68-34bc-4972-be26-90207f61f1dd-0&pdp_ext_f=%7B%22sku_id%22%3A%2212000027049416962%22%7D&pdp_npi=2%40dis%21USD%214.85%213.64%21%21%21%21%21%402100bddf16706926834816111ea09b%2112000027049416962%21sea&curPageLogUid=NP6PQAaPjqLN" target="_blank">TFT IPS 1.69"</a><br>
            <a href="https://www.aliexpress.com/item/32788923016.html" target="_blank">OLED 0.91"</a>
        </td>
        <td>Main 240x280px<br>Low power always-on</td>
    </tr>
       <tr><td><b>IO
       </b></td>
       <td>
            4-way Joystick and 2 Buttons
      <br>SD Card Reader<br>16 I/O Pins<br>Web Interface</td>
       <td><br><br>8 from Multiplexer, 8 from ESP32-S3<br><br>working on custom, versatile local server
       </td></tr>
       <tr><td><b>Misc
       </b></td>
       <td>
            <a href="https://de.aliexpress.com/item/1005003167479036.html?spm=a2g0o.detail.0.0.7cd27d94yKM3Xt&gps-id=pcDetailTopMoreOtherSeller&scm=1007.40050.362094.0&scm_id=1007.40050.362094.0&scm-url=1007.40050.362094.0&pvid=2f8b99e9-636b-44b3-80c3-0bcebe309f86&_t=gps-id%3ApcDetailTopMoreOtherSeller%2Cscm-url%3A1007.40050.362094.0%2Cpvid%3A2f8b99e9-636b-44b3-80c3-0bcebe309f86%2Ctpp_buckets%3A668%232846%238116%232002&pdp_npi=4%40dis%21EUR%2110.60%216.57%21%21%2111.33%21%21%40211b801917005985738637751e312d%2112000024821022749%21rec%21DE%213219523542%21&gatewayAdapt=glo2deu" target="_blank">5V Blower Fan</a><br>Quiet Air Duct 
       </td>
       <td> 1,5CFM, -25dbA, 8500RMP
       <br>Eval. work in progress</td>
    </tr>
    </tbody>
</table>
</br>
<hr style='height:1px'></hr>
<p><h2>Current Sensors</h2></p> 
<a href="https://www.bosch-sensortec.com/news-and-stories/stories/like-a-sniffer-dog.html">BME688:</a> Wide range of trained gases, Temperature, Humidity, Pressure <br>
<a href="https://sensirion.com/products/catalog/SCD41/">SCD41:</a> CO2 (Carbon Dioxide), Temperature, Humidity  <br>
<a href="https://sensirion.com/products/catalog/SGP41/">SGP41:</a> VOC (Volatile Organic Compounds), NOx (Nitrous Oxides) <br>

<p><h2>Potential future sensors and detections</h2></p> 
<a href="https://www.mouser.de/ProductDetail/Infineon-Technologies/PASCO2V01BUMA1?qs=tlsG%2FOw5FFi%2F2GzH4aKc4g%3D%3D">PASCO2:</a> CO2 <br>
<a href="https://www.sciosense.com/products/environmental-sensors/ens16x-digital-metal-oxide-multi-gas-sensor/">ENS161:</a> Various AQIs <br>
<a href="https://smart-nanotubes.com">Smart NanoTubes:</a> (cutting edge sensor) <br>
Carbon Monoxide (CO) <br>
Oxygen (O2) <br>
Ozone (O3) <br>
VSCs (Volatile Sulfuric Compounds) <br>
Ethanol, Benzine, Alcohol <br>
Formaldehyde (CH2O) <br>
Ammonia (NH3) <br>
UV </br>
</br>


<hr style='height:1px'></hr>
<!-- <p><h2>Summary of hazardous air compounds</h2></p> -->

![Overview Environmental Threats](https://github.com/EmanuelBender/HEX_POD/blob/main/images/Overview/Overview%20Environmental%20Threats.jpg)

</br>
<hr style='height:1px'></hr>
<h2>Local Web Interface [wip]</h2>

<h3>MAIN</h3>
<img src="https://github.com/EmanuelBender/HEX_POD/blob/main/images/Web%20Interface/MAIN.jpg"></img> 
<h3>AIR</h3>
<img src="https://github.com/EmanuelBender/HEX_POD/blob/main/images/Web%20Interface/AIR.jpg"></img> 
<h3>SYS</h3>
<img src="https://github.com/EmanuelBender/HEX_POD/blob/main/images/Web%20Interface/SYS.jpg"></img> 
<h3>LOG</h3>
<img src="https://github.com/EmanuelBender/HEX_POD/blob/main/images/Web%20Interface/LOG.jpg"></img> 
<br>
<br>

<hr style='height:1px'></hr>

<p><h2>Where it came from</h2></p><br/>
<p>
In 2019 i went to India where i travelled on Foot, Train and ultimately Motorbike from Kolkata to Kerala back up to Dharamshala and everywhere in between. So across the whole country twice.
<br>
<br>
</p><p>
Around March 2020 I was spending my time with friends in Rishikesh, one of whom slowly got really sick, right when COVID really started in India. Unfortunately she wasn't being very cooperative with receiving help despite us trying nicely. We pondered bringing her to the Hospital but couldnt really tell, no doctors to find and the medicine shops gave us wrong or fake antibiotics. We were pretty sure it wasn't Covid, but thats about it.
</p><p>
So, during that time we had the need for a thermometer but couldn't find any after searching whole Tapovan (Rishikesh). Her condition worstened and nothing helped, so after a lot of convincing and back and forth, we brought her to the Rishikesh Hospital, masked up and evading the doctors in the entrances, who were to trying to catch and test all the "Covid Tourists". We found out that she had a bladder infection that had started spreading to her kidneys. 
She stayed in the hospital for 4 days, we visited, evading doctors got harder each time. 
</p><p>
That gave me this idea, what if I could build a thing that had say, Body Temperature and HeartRate monitor, a Powerbank and a Lamp. We definitely wouldn't have been so clueless and unaware of the danger that she had been in. Our friend slowly got better with the help of real antibiotics. Right after that, we had to make the decision to leave from Delhi for Germany with the last Government chartered ariplane, which we almost missed. <br/>It was a crazy adventure.
</p><p>
Back in Germany i started planning. Very little know how about engineering, coding etc. but i was getting there, slowly but surely. 
</p><br><p>
Fast forward 3 years and i'm deep into it and still highly motivated, since i'm building this thing for myself and my vision of how its supposed to work first.<br/>
I have been working on it on and off, because money has to be made as well.
</p><p>
<br>
<br>
</p>

__________________________________________________________________________
<p align=center><b>if you are interested in supporting or joining me, write a comment or drop me a message, or</b><br>
<a href="https://www.buymeacoffee.com/ebender">buy me a coffee</a>
</p>
