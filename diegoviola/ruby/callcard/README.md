# About

A FreeSWITCH Calling Card Application

# Requirements

* ruby (>= 1.9)
* freeswitch (w/ mod_nibblebill)
* freeswitcher (>= 0.4.2)
* sequel

# Installation

  Checking for and Installing Ruby

    $ ruby --version

  If you see output similar to this, you are good to go:

    ruby 1.9.1p243 (2009-07-16 revision 24175) [i686-linux]

  Install FreeSWITCH (w/ mod_nibblebill)

    http://wiki.freeswitch.org/wiki/Installation_Guide
    http://wiki.freeswitch.org/wiki/Mod_nibblebill#Installation_and_configuration

  Install FreeSWITCHeR

    $ gem install freeswitcher

  Install Sequel

    $ gem install sequel

  Create the database schema with:

    $ ruby create_table.rb

  Load the rates on the database:

    $ ruby load_rates.rb

  Start the server/daemon with:

    $ ruby server.rb

  Configure your dialplan and point it to the application/server.

    <extension name="freeswitch-card">
      <condition field="destination_number" expression="^8084$">
        <action application="socket" data="127.0.0.1:8084 sync full"/>
      </condition>
    </extension>

# Usage

  To add a card simply fill the cards table with some data.

