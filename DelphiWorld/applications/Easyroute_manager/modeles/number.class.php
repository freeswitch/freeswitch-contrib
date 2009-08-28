<?php


    class Number
    {


        public function __construct()
        {

        }

        public function countNumbers()
        {
            // Compte le nombre de news valide
            $sql     = "SELECT COUNT(*) FROM numbers";

            $requete = mysql_query($sql) or trigger_error( mysql_error(), E_USER_ERROR);
            $data    = mysql_fetch_row($requete);

            return (int) $data[0];
        }

        public function deleteNumber($id)
        {
            // Compte le nombre de news valide
            $sql     = "DELETE FROM numbers WHERE number_id = ".(int) $id;

            $requete = mysql_query($sql) or trigger_error( mysql_error(), E_USER_ERROR);


            return true;
        }


        public function getAllNumbers($debut, $limite)
        {
            $debut  = (int) $debut  ;
            $limite = (int) $limite ;

            $sql = "SELECT number_id , gateway_id , number , acctcode , translated FROM numbers
                    ORDER by number_id DESC LIMIT ".$debut.", ".$limite;

            $requete = mysql_query($sql) or trigger_error( mysql_error(), E_USER_ERROR);
            return $requete;
        }



        public function getNumber($id_number)
        {

            $sql = "SELECT number_id , gateway_id , number , acctcode , translated FROM numbers
                    WHERE number_id = ".(int) $id_number;


            $requete = mysql_query($sql) or trigger_error( mysql_error(), E_USER_ERROR);

            $data = mysql_fetch_assoc($requete);


            return $data;
        }


        public function addNumber($gateway_id, $number, $acctcode, $translated)
        {
            $gateway_id   = (int)  $gateway_id;

            $sql = 'INSERT INTO numbers SET
                     number_id      =    ""                           ,
                     gateway_id     =    '.  $gateway_id   .'         ,
                     number         =    "'.  $number.'"              ,
                     acctcode       =    "'.  $acctcode  .'"          ,
                     translated     =    "'.  $translated .'"          ';

            $requete = mysql_query($sql) or trigger_error( mysql_error(), E_USER_ERROR);


            return (int) mysql_insert_id();
        }

        public function updateGateway($id_number, $gateway_id, $number, $acctcode, $translated)
        {
            $gateway_id   = (int)  $gateway_id;

            $sql = 'UPDATE numbers SET
                     gateway_id      =    "'.  $gateway_id   .'"      ,
                     number         =    "'.  $number.'"             ,
                     acctcode        =    "'.  $acctcode  .'"         ,
                     translated      =    "'.  $translated .'"
                    WHERE number_id = '.(int) $id_number;

            $requete = mysql_query($sql) or trigger_error( mysql_error(), E_USER_ERROR);

            return true;
        }



    }