<?php
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                                easyroute

    Derniere mise a jour  :  22 aout  09
    Auteur                : InSiDe
    Dev 0.1
    ------------------------------------------------------------------------------*/


    include CHEMIN_MODELE. 'number.class.php';


    if (empty($_GET['id']))
        trigger_error("ID du number introuvable.", E_USER_ERROR);

    $id = (int) $_GET['id'];


    // Instanciation de la classe Commentaires
    $numer = new Number();

    if($numer->deleteNumber($id))
        header('location:./index.php?module=numbers&action=liste_numbers');
    else
        trigger_error("Erreur SQL impossible de supprimer le number", E_USER_ERROR);



?>