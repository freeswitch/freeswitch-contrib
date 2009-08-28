<?php
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                                easyroute

    Derniere mise a jour  :  22 aout  09
    Auteur                : InSiDe
    Dev 0.1
    ------------------------------------------------------------------------------*/


    include CHEMIN_MODELE. 'gateway.class.php';


    if (empty($_GET['id']))
        trigger_error("ID du gateway introuvable.", E_USER_ERROR);

    $id = (int) $_GET['id'];


    // Instanciation de la classe Commentaires
    $gateway = new Gateway();

    if($gateway->deleteGateway($id))
        header('location:./index.php?module=gateway&action=liste_gateways');
    else
        trigger_error("Erreur SQL impossible de supprimer le gateway", E_USER_ERROR);



?>