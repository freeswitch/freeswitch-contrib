<?php
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                                eduDz

    Derniere mise a jour  :  Juin 09
    Auteur                : InSiDe
    Dev 1.7 MVC Phoenix Rising (Commencée le 28 Mai 2009)

    ------------------------------------------------------------------------------*/

    require_once 'global/config.php';



    // Utilisation et démarrage des sessions
    session_start();

    // On se connecte à la BDD
    mysql_connect(SQL_DSN, SQL_USERNAME, SQL_PASSWORD)
                  or trigger_error('Impossible de se connecter à la base de donnée', E_USER_ERROR);

    mysql_select_db(SQL_BDD);
    mysql_query ('SET NAMES utf8');


    // Pour que la date soit en francais ^^ setLocale(LC_TIME, 'fr_FR', 'fra');
    setLocale(LC_TIME, 'fr_FR.utf8', 'fra.utf-8');


?>