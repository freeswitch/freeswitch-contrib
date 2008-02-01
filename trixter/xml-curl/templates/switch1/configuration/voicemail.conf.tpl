<configuration name="voicemail.conf" description="Voicemail">
  <settings>
  </settings>
  <profiles>
    <profile name="default">
      <param name="file-extension" value="wav"/>
      <param name="terminator-key" value="#"/>
      <param name="max-login-attempts" value="3"/>
      <param name="digit-timeout" value="10000"/>
      <param name="max-record-len" value="300"/>
      <param name="tone-spec" value="%(1000, 0, 640)"/>
      <param name="callback-dialplan" value="XML"/>
      <param name="callback-context" value="default"/>
      <param name="terminator-key" value="#"/>
      <param name="play-new-messages-key" value="1"/>
      <param name="play-saved-messages-key" value="2"/>
      <param name="main-menu-key" value="0"/>
      <param name="config-menu-key" value="5"/>
      <param name="record-greeting-key" value="1"/>
      <param name="choose-greeting-key" value="2"/>
      <param name="record-name-key" value="3"/>
      <param name="record-file-key" value="3"/>
      <param name="listen-file-key" value="1"/>
      <param name="save-file-key" value="2"/>
      <param name="delete-file-key" value="7"/>
      <param name="undelete-file-key" value="8"/>
      <param name="email-key" value="4"/>
      <param name="pause-key" value="0"/>
      <param name="restart-key" value="1"/>
      <param name="ff-key" value="6"/>
      <param name="rew-key" value="4"/>
      <email>
        <body>{literal}<![CDATA[At ${voicemail_time} you were left a message from ${voicemail_caller_id_name} ${voicemail_caller_id_number}
to your account ${voicemail_account}@${voicemail_domain}
]]>{/literal}</body>
        <headers>{literal}<![CDATA[From: FreeSWITCH mod_voicemail <${voicemail_account}@${voicemail_domain}>
To: <${voicemail_email}>
Subject: Voicemail from ${voicemail_caller_id_name} ${voicemail_caller_id_number}
X-Priority: ${voicemail_priority}
X-Mailer: FreeSWITCH
]]>{/literal}</headers>
        <!-- this is the format voicemail_time will have -->
        <param name="date-fmt" value="%A, %B %d %Y, %I %M %p"/>
        <param name="email-from" value="{literal}${voicemail_account}@${voicemail_domain}{/literal}"/>
      </email>
      <!--<param name="storage-dir" value="/tmp"/>-->
      <!--<param name="odbc-dsn" value="dsn:user:pass"/>-->
    </profile>
  </profiles>
</configuration> 
