<?php
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                                easyroute

    Derniere mise a jour  :  22 aout  09
    Auteur                : InSiDe
    Dev 0.1
    ------------------------------------------------------------------------------*/


    include CHEMIN_MODELE. 'number.class.php';


    if(isset($_GET['id']))
        $id_gate = (int) $_GET['id'];
    else
        $id_gate = null;

    if (!empty($_POST['uniqid']))
    {

        if(empty($_POST['gateway_id']) )
            $erreurs[] = "gateway_id est vide.";

        if(empty($_POST['number']) )
            $erreurs[] = "number est vide.";

        if(empty($_POST['acctcode']) )
            $erreurs[] = "acctcode est vide.";

        if(empty($_POST['translated']) )
            $erreurs[] = "translated est vide.";

        if(empty($erreurs))
        {
            $gateway      = htmlspecialchars($_POST['gateway_id'], ENT_COMPAT, 'UTF-8');
            $gateway      = mysql_real_escape_string($gateway);

            $number       = htmlspecialchars($_POST['number'], ENT_COMPAT, 'UTF-8');
            $number       = mysql_real_escape_string($number);

            $acctcode     = htmlspecialchars($_POST['acctcode'], ENT_COMPAT, 'UTF-8');
            $acctcode     = mysql_real_escape_string($limit);

            $translated   = htmlspecialchars($_POST['translated'], ENT_COMPAT, 'UTF-8');
            $translated   = mysql_real_escape_string($techprofile);


            $gateway = new Number();

            if($gateway->addNumber($gateway, $number, $acctcode, $translated))
                header('location:./index.php?module=numbers&action=liste_numbers');
            else
                trigger_error("Erreur SQL impossible d'ajouter un nouv. Number", E_USER_ERROR);


        }
    }

    // Affichage
    include CHEMIN_VUE. 'formulalre_ajouter_number.tpl.php';

?>