<?php
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                                easyroute

    Derniere mise a jour  :  22 aout  09
    Auteur                : InSiDe
    Dev 0.1
    ------------------------------------------------------------------------------*/


    include CHEMIN_MODELE. 'gateway.class.php';


    if (!empty($_POST['uniqid']))
    {

        if(empty($_POST['gateway_ip']) )
            $erreurs[] = "gateway_ip est vide.";

        if(empty($_POST['group']) )
            $erreurs[] = "group est vide.";

        if(empty($_POST['limit']) )
            $erreurs[] = "limit est vide.";

        if(empty($_POST['techprofile']) )
            $erreurs[] = "Techprofile est vide.";

        if(empty($erreurs))
        {
            $ip          = htmlspecialchars($_POST['gateway_ip'], ENT_COMPAT, 'UTF-8');
            $ip          = mysql_real_escape_string($ip);

            $group       = htmlspecialchars($_POST['group'], ENT_COMPAT, 'UTF-8');
            $group       = mysql_real_escape_string($group);

            $limit       = htmlspecialchars($_POST['limit'], ENT_COMPAT, 'UTF-8');
            $limit       = mysql_real_escape_string($limit);

            $techprofile = htmlspecialchars($_POST['techprofile'], ENT_COMPAT, 'UTF-8');
            $techprofile = mysql_real_escape_string($techprofile);

            // Instanciation de la classe Commentaires
            $gateway = new Gateway();

            if($gateway->addGateway($ip, $group, $limit, $techprofile))
                header('location:./index.php?module=gateway&action=liste_gateways');
            else
                trigger_error("Erreur SQL impossible d'ajouter un nouv. gateway", E_USER_ERROR);


        }
    }

    // Affichage
    include CHEMIN_VUE. 'formulalre_ajouter_gateway.tpl.php';

?>