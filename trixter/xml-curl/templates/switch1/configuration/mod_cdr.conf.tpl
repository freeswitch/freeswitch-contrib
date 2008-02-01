<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
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
