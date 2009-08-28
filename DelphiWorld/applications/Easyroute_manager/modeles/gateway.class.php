<?php


    class Gateway
    {


        public function __construct()
        {

        }

        public function countGateways()
        {
            // Compte le nombre de news valide
            $sql     = "SELECT COUNT(*) FROM gateways";

            $requete = mysql_query($sql) or trigger_error( mysql_error(), E_USER_ERROR);
            $data    = mysql_fetch_row($requete);

            return (int) $data[0];
        }

        public function deleteGateway($id)
        {
            // Compte le nombre de news valide
            $sql     = "DELETE FROM gateways WHERE gateway_id = ".(int) $id;
            $sql2    = "DELETE FROM numbers  WHERE gateway_id = ".(int) $id;

            $requete = mysql_query($sql) or trigger_error( mysql_error(), E_USER_ERROR);
            $requete2 = mysql_query($sql2) or trigger_error( mysql_error(), E_USER_ERROR);

            return true;
        }

        public function getAllGateways($debut, $limite)
        {
            $debut  = (int) $debut  ;
            $limite = (int) $limite ;

            $sql = "SELECT gateway_id , gateway_ip , `group` , `limit` , techprofile FROM gateways
                    ORDER by gateway_id DESC LIMIT ".$debut.", ".$limite;

            $requete = mysql_query($sql) or trigger_error( mysql_error(), E_USER_ERROR);
            return $requete;
        }



        public function getAGateway($id_gatway)
        {

            $sql = "SELECT gateway_id, gateway_ip, `group` , `limit` , techprofile
                    FROM gateways
                    WHERE gateway_id = ".(int) $id_gatway;


            $requete = mysql_query($sql) or trigger_error( mysql_error(), E_USER_ERROR);

            $data = mysql_fetch_assoc($requete);


            return $data;
        }

        public function addGateway($gateway_ip, $group, $limit, $techprofile)
        {
            $limit   = (int)  $limit;

            $sql = 'INSERT INTO gateways SET
                     gateway_id      =    ""                             ,
                     gateway_ip      =    "'.  $gateway_ip   .'"         ,
                     `group`         =    "'.  $group.'"                 ,
                     `limit`         =    '.  $limit  .'                 ,
                     techprofile     =    "'.  $techprofile .'"          ';

            $requete = mysql_query($sql) or trigger_error( mysql_error(), E_USER_ERROR);


            return (int) mysql_insert_id();
        }

        public function updateGateway($id_gatway, $gateway_ip, $group, $limit, $techprofile)
        {
            $limit   = (int)  $limit;

            $sql = 'UPDATE gateways SET
                     gateway_ip      =    "'.  $gateway_ip   .'"        ,
                     `group`         =    "'.  $group.'"                ,
                     `limit`         =    '.  $limit  .'                ,
                     techprofile     =    "'.  $techprofile .'"
                    WHERE gateway_id = '.(int) $id_gatway;

            $requete = mysql_query($sql) or trigger_error( mysql_error(), E_USER_ERROR);

            return true;
        }



    }