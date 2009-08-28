
   <h2> edit number</h2>

    <a href="./index.php?module=numbers&action=liste_numbers">Manage numbers</a>
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
            <label for="gateway_id">gateway ID :</label> <br />
            <input type="text" size="80" id="gateway_id" name="gateway_id" value="<?php echo $data['gateway_id']?>" />
        </p>

        <p>
            <label for="number">Number : </label><br />
            <input type="text" size="80" id="number" name="number"  value="<?php echo $data['number']?>"/>
        </p>

        <p>
            <label for="acctcode">Acctcode :</label> <br />
            <input type="text" size="80" id="acctcode" name="acctcode"  value="<?php echo $data['acctcode']?>" />
        </p>


        <p>
            <label for="translated">Translated :</label> <br />
            <input type="text" size="80" id="translated" name="translated" value="<?php echo $data['translated']?>"/>
        </p>


        <input type="hidden" name="uniqid" value="02AXDZ4B"/>
        <p>
            <input type="submit" value="Envoyer"  />
        </p>


        </div>
    </form>