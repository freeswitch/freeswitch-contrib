<?php

class Welcome extends Controller {

	
function Welcome()
	{
		parent::Controller();	
	}
	
	function index()
	{
	
	$this->load->view('nav');

	$this->load->view('welcome_message');
	$this->load->view('fouter');
}
}

/* End of file welcome.php */
/* 

	}
}

/* End of file welcome.php 
Location: ./system/application/controllers/welcome.php */