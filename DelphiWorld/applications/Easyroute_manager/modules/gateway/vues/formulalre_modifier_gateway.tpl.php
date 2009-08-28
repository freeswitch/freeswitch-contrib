
   <h2> Edit Gateway </h2>

    <a href="./index.php?module=gateway&action=liste_gateways">Gateways List</a>

    <br /> <br />

    <p>&nbsp;</p>
    <?php
    if (!empty($erreurs))
    {
        echo ' <ul>'."\n";

        foreach($erreurs as $e)
            echo '<li>'. $e .'</li>'."\n";

        echo '</ul>';
    }
    ?>

    <form method="post" name="formulaire">

        <p>
            <label for="gateway_ip">IP :</label> <br />
            <input type="text" size="80" id="gateway_ip" name="gateway_ip" value="<?php echo $data['gateway_ip'] ?>" />
        </p>
        <p>
            <label for="group">Groupe :</label>  <br />
            <input type="text" size="80" id="group" name="group"  maxlength="255" value="<?php echo $data['group'] ?>"/>
        </p>

        <p>
            <label for="limit">Limite : </label> <br />
            <input type="text" size="80" id="limit" name="limit"  maxlength="255" value="<?php echo $data['limit'] ?>"/>
        </p>


        <p>
            <label for="techprofile">Techprofile : </label> <br />
            <textarea name="techprofile" id="techprofile" cols="50" rows="16"><?php echo $data['techprofile'] ?>"</textarea>
        </p>

        <input type="hidden" name="uniqid" value="02XA4B"/>
        <p>
            <input type="submit" value="Envoyer"  />
        </p>

        </div>
    </form>