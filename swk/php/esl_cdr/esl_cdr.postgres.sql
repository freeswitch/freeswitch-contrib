--
-- PostgreSQL database dump
--

SET client_encoding = 'UTF8';
SET standard_conforming_strings = off;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET escape_string_warning = off;

SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: cdr; Type: TABLE; Schema: public; Owner: root; Tablespace: 
--

CREATE TABLE cdr (
    uuid character varying,
    acctcode character varying,
    ani character varying,
    e164_ani character varying,
    dnis character varying,
    translated_dnis character varying,
    e164_dnis character varying,
    src_host character varying,
    read_codec character varying,
    write_codec character varying,
    clid_name character varying,
    clid_number character varying,
    src_sdp character varying,
    dest_sdp character varying,
    originate_disposition character varying,
    bridge_to_uuid character varying,
    endpoint_disposition character varying,
    sip_hangup_disposition character varying,
    term_cause character varying,
    hangup_cause character varying,
    start_stamp timestamp with time zone,
    answer_stamp timestamp with time zone,
    progress_media_stamp timestamp with time zone,
    end_stamp timestamp with time zone,
    duration integer,
    billsec integer,
    progress_mediasec integer,
    billmin numeric(8,2),
    carrier_id integer,
    cust_cost numeric(7,4),
    carrier_cost numeric(7,4)
);


ALTER TABLE public.cdr OWNER TO root;

--
-- Name: public; Type: ACL; Schema: -; Owner: postgres
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;


--
-- PostgreSQL database dump complete
--

