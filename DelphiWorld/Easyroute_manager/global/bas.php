</div>


    <div id="pied_de_page">
        <div class="flot_gauche">
         <br /> <br />
        <?php
echo ('copyright ©(C) 2009 Freeswitch.org');
echo ('<p></p>');
            $temps_fin =  microtime(true);
            $temps_execution = round($temps_fin - $temps_debut,4);

            $temps_execution = "Version 0.1 |randred  in : ".($temps_execution * 1000).' ms ';

            echo $temps_execution;

        ?>


    </div>

</body>

</html>
