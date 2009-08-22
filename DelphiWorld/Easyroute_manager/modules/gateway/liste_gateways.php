<?php
    require CHEMIN_LIB.   'fonction.paigner.php';
    include CHEMIN_MODELE. 'gateway.class.php';

    $head['titre'] = 'Manage Gateways.';

    // Instanciation de la classe Commentaires
    $gateway = new Gateway();

    // Récupération du nombre de commentaires
    $nombre_gateways = $gateway->countGateways();

    // Préparation de la paignation
    $lien_paignation = ' <a href="./index.php?module=gateway&action=liste_gateways&page=%d"> %1$d </a> ';
    $paignation = paigner($nombre_gateways, GATEWAYS_PAR_PAGE, $page, $lien_paignation);

    // Récupération des commentaires
    $premiereGatway = ($page -1) * GATEWAYS_PAR_PAGE;
    $requete = $gateway->getAllGateways($premiereGatway, GATEWAYS_PAR_PAGE);

    // Affichage
    include CHEMIN_VUE. 'en-tete_liste_gateways.tpl.php';

    while($data = mysql_fetch_assoc($requete))
        include CHEMIN_VUE. 'colonne_gateway.tpl.php';

    include CHEMIN_VUE. 'bas_liste_gateways.tpl.php';

?>