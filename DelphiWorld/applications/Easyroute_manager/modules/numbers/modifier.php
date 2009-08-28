<?php
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                                easyroute

    Derniere mise a jour  :  22 aout  09
    Auteur                : InSiDe
    Dev 0.1
    ------------------------------------------------------------------------------*/


    include CHEMIN_MODELE. 'number.class.php';

    if(empty($_GET['id']))
        trigger_error("ID du number introuvable.", E_USER_ERROR);

    $id_number = (int) $_GET['id'];

    if (!empty($_POST['uniqid']))
    {

        if(!isset($_POST['gateway_id']) )
            $erreurs[] = "gateway_ip est inexistant.";

        if(!isset($_POST['number']) )
            $erreurs[] = "number est inexistant.";

        if(!isset($_POST['acctcode']) )
            $erreurs[] = "acctcode est inexistant.";

        if(!isset($_POST['translated']) )
            $erreurs[] = "translated est inexistant.";

        if(empty($erreurs))
        {
            $gateway      = htmlspecialchars($_POST['gateway_id'], ENT_COMPAT, 'UTF-8');
            $gateway      = mysql_real_escape_string($gateway);

            $number       = htmlspecialchars($_POST['number'], ENT_COMPAT, 'UTF-8');
            $number       = mysql_real_escape_string($number);

            $acctcode     = htmlspecialchars($_POST['acctcode'], ENT_COMPAT, 'UTF-8');
            $acctcode     = mysql_real_escape_string($acctcode);

            $translated   = htmlspecialchars($_POST['translated'], ENT_COMPAT, 'UTF-8');
            $translated   = mysql_real_escape_string($translated);

            // Instanciation Number
            $numer = new Number();

            if($numer->updateNumber($id_number, $gateway, $number, $acctcode, $translated))
                header('location:./index.php?module=numbers&action=liste_numbers');
            else
                trigger_error("Erreur SQL impossible de modifier le number", E_USER_ERROR);


        }
    }

    $numer = new Number();
    $data    = $numer->getNumber($id_number);
    // Affichage
    include CHEMIN_VUE. 'formulalre_modifier_number.tpl.php';

?>