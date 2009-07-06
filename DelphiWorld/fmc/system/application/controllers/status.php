<?php
class status extends Controller {
	function index()
	{
$this->load->view('nav');
$this->load->view('status');
echo ('<p></p>');
$this->load->view('fouter');
	}
}
?>