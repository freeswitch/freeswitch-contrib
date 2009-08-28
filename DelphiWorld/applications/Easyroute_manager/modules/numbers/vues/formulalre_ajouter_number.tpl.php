
   <h2> add a new number</h2>

    <a href="./index.php?module=numbers&action=liste_numbers">Manage Numbers</a>

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
            <label for="gateway_id">gateway ID:</label> <br />
            <input type="text" size="80" id="gateway_id" name="gateway_id" value="<?php echo $id_gate?>" maxlength="255"/>
        </p>
        <p>
            <label for="number">Number : </label><br />
            <input type="text" size="80" id="number" name="number"  maxlength="255"/>
        </p>

        <p>
            <label for="acctcode">Acctcode :</label> <br />
            <input type="text" size="80" id="acctcode" name="acctcode"  maxlength="255"/>
        </p>


        <p>
            <label for="translated">Translated :</label> <br />
            <input type="text" size="80" id="translated" name="translated"  maxlength="255"/>
        </p>


        <input type="hidden" name="uniqid" value="02AXDZ4B"/>
        <p>
            <input type="submit" value="save"  />
        </p>

        </div>
    </form>