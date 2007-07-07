<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
    <configuration name="ivr.conf" description="IVR menus">
      <menus>
        <menu name="main"
              greet-long="/soundfiles/greet-long.wav"
              greet-short="/soundfiles/greet-short.wav"
              invalid-sound="/soundfiles/invalid.wav"
              exit-sound="/soundfiles/exit.wav" timeout ="15" max-failures="3">
          <entry action="menu-exit" digits="*"/>
          <entry action="menu-sub" digits="2" param="menu2"/>
          <entry action="menu-exec-api" digits="3" param="api arg"/>
          <entry action="menu-play-sound" digits="4" param="/soundfiles/4.wav"/>
          <entry action="menu-back" digits="5"/>
          <entry action="menu-call-transfer" digits="7" param="888"/>
          <entry action="menu-sub" digits="8" param="menu8"/>>
        </menu>
        <menu name="menu8"
              greet-long="/soundfiles/greet-long.wav"
              greet-short="/soundfiles/greet-short.wav"
              invalid-sound="/soundfiles/invalid.wav"
              exit-sound="/soundfiles/exit.wav"
              timeout ="15"
              max-failures="3">
          <entry action="menu-back" digits="#"/>
          <entry action="menu-play-sound" digits="4" param="/soundfiles/4.wav"/>
          <entry action="menu-top" digits="*"/>
        </menu>
        <menu name="menu2"
              greet-long="/soundfiles/greet-long.wav"
              greet-short="/soundfiles/greet-short.wav"
              invalid-sound="/soundfiles/invalid.wav"
              exit-sound="/soundfiles/exit.wav"
              timeout ="15"
              max-failures="3">
          <entry action="menu-back" digits="#"/>
          <entry action="menu-play-sound" digits="4" param="/soundfiles/4.wav"/>
          <entry action="menu-top" digits="*"/>
        </menu>
      </menus>
    </configuration>
    <configuration name="mod_cdr.conf" description="CDR Configuration">
      <pddcdr>
        <param name="path" value="/work/temp/pddcdr"/>
        <!-- <param name="chanvars" value="username,accountcode"/> -->
        <!-- <param name="chanvars_fixed" value="foo"/> -->
      </pddcdr>
      <csvcdr>
        <param name="path" value="/work/temp/csvcdr"/>
        <param name="size_limit" value="25"/>
        <!-- <param name="chanvars_fixed" value="username"/> -->
        <!-- <param name="chanvars_supp" value="*"/> -->
        <!-- <param name="repeat_fixed_in_supp" value="0"/> -->
      </csvcdr>
      <mysqlcdr>
        <param name="hostname" value="10.0.0.1"/>
        <param name="username" value="test"/>
        <param name="password" value="test"/>
        <param name="dbname" value="testing"/>
        <!-- This following line logs username as a varchar, and The_Kow as a tiny -->
        <!-- <param name="chanvars_fixed" value="username=s,The_Kow=t"/> -->
        <!-- If you do not want to log any and all chanvars to the chanvar table, comment the next 2 lines out -->
        <param name="chanvars_supp" value="*"/>
        <param name="chanvars_supp_repeat_fixed" value="0"/>
      </mysqlcdr>
    </configuration>
  </section>
</document>
