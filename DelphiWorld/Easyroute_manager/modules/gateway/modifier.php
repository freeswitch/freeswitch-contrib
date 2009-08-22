<?php
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                                easyroute

    Derniere mise a jour  :  22 aout  09
    Auteur                : InSiDe
    Dev 0.1
    ------------------------------------------------------------------------------*/


    include CHEMIN_MODELE. 'gateway.class.php';

    if(empty($_GET['id']))
        trigger_error("ID du number gateway.", E_USER_ERROR);

    $id_gateway = (int) $_GET['id'];

    if (!empty($_POST['uniqid']))
    {

        if(!isset($_POST['gateway_ip']) )
            $erreurs[] = "gateway_ip est inexistant.";

        if(!isset($_POST['group']) )
            $erreurs[] = "group est inexistant.";

        if(!isset($_POST['limit']) )
            $erreurs[] = "limit est inexistant.";

        if(!isset($_POST['techprofile']) )
            $erreurs[] = "limit est inexistant.";

        if(empty($erreurs))
        {
            $ip      = htmlspecialchars($_POST['gateway_ip'], ENT_COMPAT, 'UTF-8');
            $ip      = mysql_real_escape_string($ip);

            $group       = htmlspecialchars($_POST['group'], ENT_COMPAT, 'UTF-8');
            $group       = mysql_real_escape_string($group);

            $limit     = htmlspecialchars($_POST['limit'], ENT_COMPAT, 'UTF-8');
            $limit     = mysql_real_escape_string($limit);

            $techprofile   = htmlspecialchars($_POST['techprofile'], ENT_COMPAT, 'UTF-8');
            $translated   = mysql_real_escape_string($translated);

            // Instanciation Number
            $getway = new Gateway();

            if($getway->updateGateway($id_gateway, $ip, $group, $limit, $techprofile))
                header('location:./index.php?module=getway&action=liste_getways');
            else
                trigger_error("Erreur SQL impossible de modifier le getway", E_USER_ERROR);


        }
    }

    $getway = new Gateway();
    $data    = $getway->getAGateway($id_gateway);
    // Affichage
    include CHEMIN_VUE. 'formulalre_modifier_gateway.tpl.php';

?>