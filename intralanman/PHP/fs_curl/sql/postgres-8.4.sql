--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = off;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET escape_string_warning = off;

--
-- Name: plpgsql; Type: PROCEDURAL LANGUAGE; Schema: -; Owner: postgres
--

CREATE PROCEDURAL LANGUAGE plpgsql;


ALTER PROCEDURAL LANGUAGE plpgsql OWNER TO postgres;

SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: acl_lists; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE acl_lists (
    id integer NOT NULL,
    acl_name character varying(128) NOT NULL,
    default_policy character varying(48) NOT NULL
);


ALTER TABLE public.acl_lists OWNER TO freeswitch;

--
-- Name: acl_lists_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE acl_lists_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.acl_lists_id_seq OWNER TO freeswitch;

--
-- Name: acl_lists_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE acl_lists_id_seq OWNED BY acl_lists.id;


--
-- Name: acl_lists_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('acl_lists_id_seq', 1, false);


--
-- Name: acl_nodes; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE acl_nodes (
    id integer NOT NULL,
    cidr character varying(48) NOT NULL,
    type character varying(16) NOT NULL,
    list_id integer NOT NULL
);


ALTER TABLE public.acl_nodes OWNER TO freeswitch;

--
-- Name: acl_nodes_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE acl_nodes_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.acl_nodes_id_seq OWNER TO freeswitch;

--
-- Name: acl_nodes_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE acl_nodes_id_seq OWNED BY acl_nodes.id;


--
-- Name: acl_nodes_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('acl_nodes_id_seq', 1, false);


--
-- Name: carrier_gateway; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE carrier_gateway (
    id integer NOT NULL,
    carrier_id integer,
    prefix character varying(128) DEFAULT ''::character varying NOT NULL,
    suffix character varying(128) DEFAULT ''::character varying NOT NULL,
    codec character varying(128) DEFAULT ''::character varying NOT NULL,
    enabled boolean DEFAULT true NOT NULL
);


ALTER TABLE public.carrier_gateway OWNER TO freeswitch;

--
-- Name: carrier_gateway_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE carrier_gateway_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.carrier_gateway_id_seq OWNER TO freeswitch;

--
-- Name: carrier_gateway_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE carrier_gateway_id_seq OWNED BY carrier_gateway.id;


--
-- Name: carrier_gateway_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('carrier_gateway_id_seq', 1, false);


--
-- Name: carriers; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE carriers (
    id integer NOT NULL,
    carrier_name character varying(255) NOT NULL,
    enabled boolean DEFAULT true NOT NULL
);


ALTER TABLE public.carriers OWNER TO freeswitch;

--
-- Name: carriers_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE carriers_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.carriers_id_seq OWNER TO freeswitch;

--
-- Name: carriers_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE carriers_id_seq OWNED BY carriers.id;


--
-- Name: carriers_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('carriers_id_seq', 1, false);


--
-- Name: conference_advertise; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE conference_advertise (
    id integer NOT NULL,
    room character varying(64) NOT NULL,
    status character varying(128) NOT NULL
);


ALTER TABLE public.conference_advertise OWNER TO freeswitch;

--
-- Name: conference_advertise_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE conference_advertise_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.conference_advertise_id_seq OWNER TO freeswitch;

--
-- Name: conference_advertise_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE conference_advertise_id_seq OWNED BY conference_advertise.id;


--
-- Name: conference_advertise_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('conference_advertise_id_seq', 1, false);


--
-- Name: conference_controls; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE conference_controls (
    id integer NOT NULL,
    conf_group character varying(64) NOT NULL,
    action character varying(64) NOT NULL,
    digits character varying(16) NOT NULL
);


ALTER TABLE public.conference_controls OWNER TO freeswitch;

--
-- Name: conference_controls_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE conference_controls_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.conference_controls_id_seq OWNER TO freeswitch;

--
-- Name: conference_controls_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE conference_controls_id_seq OWNED BY conference_controls.id;


--
-- Name: conference_controls_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('conference_controls_id_seq', 1, false);


--
-- Name: conference_profiles; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE conference_profiles (
    id integer NOT NULL,
    profile_name character varying(64) NOT NULL,
    param_name character varying(64) NOT NULL,
    param_value character varying(64) NOT NULL
);


ALTER TABLE public.conference_profiles OWNER TO freeswitch;

--
-- Name: conference_profiles_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE conference_profiles_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.conference_profiles_id_seq OWNER TO freeswitch;

--
-- Name: conference_profiles_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE conference_profiles_id_seq OWNED BY conference_profiles.id;


--
-- Name: conference_profiles_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('conference_profiles_id_seq', 1, false);


--
-- Name: dialplan; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE dialplan (
    dialplan_id integer NOT NULL,
    domain character varying(128) NOT NULL,
    ip_address character varying(16) NOT NULL
);


ALTER TABLE public.dialplan OWNER TO freeswitch;

--
-- Name: dialplan_actions; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE dialplan_actions (
    action_id integer NOT NULL,
    condition_id integer NOT NULL,
    application character varying(256) NOT NULL,
    data character varying(256) NOT NULL,
    type character varying(32) NOT NULL,
    weight integer NOT NULL
);


ALTER TABLE public.dialplan_actions OWNER TO freeswitch;

--
-- Name: dialplan_actions_action_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE dialplan_actions_action_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.dialplan_actions_action_id_seq OWNER TO freeswitch;

--
-- Name: dialplan_actions_action_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE dialplan_actions_action_id_seq OWNED BY dialplan_actions.action_id;


--
-- Name: dialplan_actions_action_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('dialplan_actions_action_id_seq', 1, false);


--
-- Name: dialplan_condition; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE dialplan_condition (
    condition_id integer NOT NULL,
    extension_id integer NOT NULL,
    field character varying(128) NOT NULL,
    expression character varying(128) NOT NULL,
    weight integer NOT NULL
);


ALTER TABLE public.dialplan_condition OWNER TO freeswitch;

--
-- Name: dialplan_condition_condition_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE dialplan_condition_condition_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.dialplan_condition_condition_id_seq OWNER TO freeswitch;

--
-- Name: dialplan_condition_condition_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE dialplan_condition_condition_id_seq OWNED BY dialplan_condition.condition_id;


--
-- Name: dialplan_condition_condition_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('dialplan_condition_condition_id_seq', 1, false);


--
-- Name: dialplan_context; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE dialplan_context (
    context_id integer NOT NULL,
    dialplan_id integer NOT NULL,
    context character varying(64) NOT NULL,
    weight integer NOT NULL
);


ALTER TABLE public.dialplan_context OWNER TO freeswitch;

--
-- Name: dialplan_context_context_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE dialplan_context_context_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.dialplan_context_context_id_seq OWNER TO freeswitch;

--
-- Name: dialplan_context_context_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE dialplan_context_context_id_seq OWNED BY dialplan_context.context_id;


--
-- Name: dialplan_context_context_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('dialplan_context_context_id_seq', 1, false);


--
-- Name: dialplan_dialplan_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE dialplan_dialplan_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.dialplan_dialplan_id_seq OWNER TO freeswitch;

--
-- Name: dialplan_dialplan_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE dialplan_dialplan_id_seq OWNED BY dialplan.dialplan_id;


--
-- Name: dialplan_dialplan_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('dialplan_dialplan_id_seq', 1, false);


--
-- Name: dialplan_extension; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE dialplan_extension (
    extension_id integer NOT NULL,
    context_id integer NOT NULL,
    name character varying(128) NOT NULL,
    continue character varying(32) NOT NULL,
    weight integer NOT NULL
);


ALTER TABLE public.dialplan_extension OWNER TO freeswitch;

--
-- Name: dialplan_extension_extension_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE dialplan_extension_extension_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.dialplan_extension_extension_id_seq OWNER TO freeswitch;

--
-- Name: dialplan_extension_extension_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE dialplan_extension_extension_id_seq OWNED BY dialplan_extension.extension_id;


--
-- Name: dialplan_extension_extension_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('dialplan_extension_extension_id_seq', 1, false);


--
-- Name: dialplan_special; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE dialplan_special (
    id integer NOT NULL,
    context character varying(256) NOT NULL,
    class_file character varying(256) NOT NULL
);


ALTER TABLE public.dialplan_special OWNER TO freeswitch;

--
-- Name: dialplan_special_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE dialplan_special_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.dialplan_special_id_seq OWNER TO freeswitch;

--
-- Name: dialplan_special_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE dialplan_special_id_seq OWNED BY dialplan_special.id;


--
-- Name: dialplan_special_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('dialplan_special_id_seq', 1, false);


--
-- Name: dingaling_profile_params; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE dingaling_profile_params (
    id integer NOT NULL,
    dingaling_id integer NOT NULL,
    param_name character varying(64) NOT NULL,
    param_value character varying(64) NOT NULL
);


ALTER TABLE public.dingaling_profile_params OWNER TO freeswitch;

--
-- Name: dingaling_profile_params_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE dingaling_profile_params_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.dingaling_profile_params_id_seq OWNER TO freeswitch;

--
-- Name: dingaling_profile_params_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE dingaling_profile_params_id_seq OWNED BY dingaling_profile_params.id;


--
-- Name: dingaling_profile_params_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('dingaling_profile_params_id_seq', 1, false);


--
-- Name: dingaling_profiles; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE dingaling_profiles (
    id integer NOT NULL,
    profile_name character varying(64) NOT NULL,
    type character varying(64) NOT NULL
);


ALTER TABLE public.dingaling_profiles OWNER TO freeswitch;

--
-- Name: dingaling_profiles_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE dingaling_profiles_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.dingaling_profiles_id_seq OWNER TO freeswitch;

--
-- Name: dingaling_profiles_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE dingaling_profiles_id_seq OWNED BY dingaling_profiles.id;


--
-- Name: dingaling_profiles_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('dingaling_profiles_id_seq', 1, false);


--
-- Name: dingaling_settings; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE dingaling_settings (
    id integer NOT NULL,
    param_name character varying(64) NOT NULL,
    param_value character varying(64) NOT NULL
);


ALTER TABLE public.dingaling_settings OWNER TO freeswitch;

--
-- Name: dingaling_settings_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE dingaling_settings_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.dingaling_settings_id_seq OWNER TO freeswitch;

--
-- Name: dingaling_settings_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE dingaling_settings_id_seq OWNED BY dingaling_settings.id;


--
-- Name: dingaling_settings_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('dingaling_settings_id_seq', 1, false);


--
-- Name: directory; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE directory (
    id integer NOT NULL,
    username character varying(256) NOT NULL,
    domain character varying(256) NOT NULL
);


ALTER TABLE public.directory OWNER TO freeswitch;

--
-- Name: directory_domains; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE directory_domains (
    id integer NOT NULL,
    domain_name character varying(128) NOT NULL
);


ALTER TABLE public.directory_domains OWNER TO freeswitch;

--
-- Name: directory_domains_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE directory_domains_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.directory_domains_id_seq OWNER TO freeswitch;

--
-- Name: directory_domains_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE directory_domains_id_seq OWNED BY directory_domains.id;


--
-- Name: directory_domains_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('directory_domains_id_seq', 1, false);


--
-- Name: directory_gateway_params; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE directory_gateway_params (
    id integer NOT NULL,
    d_gw_id integer NOT NULL,
    param_name character varying(64) NOT NULL,
    param_value character varying(64) NOT NULL
);


ALTER TABLE public.directory_gateway_params OWNER TO freeswitch;

--
-- Name: directory_gateway_params_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE directory_gateway_params_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.directory_gateway_params_id_seq OWNER TO freeswitch;

--
-- Name: directory_gateway_params_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE directory_gateway_params_id_seq OWNED BY directory_gateway_params.id;


--
-- Name: directory_gateway_params_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('directory_gateway_params_id_seq', 1, false);


--
-- Name: directory_gateways; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE directory_gateways (
    id integer NOT NULL,
    directory_id integer NOT NULL,
    gateway_name character varying(128) NOT NULL
);


ALTER TABLE public.directory_gateways OWNER TO freeswitch;

--
-- Name: directory_gateways_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE directory_gateways_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.directory_gateways_id_seq OWNER TO freeswitch;

--
-- Name: directory_gateways_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE directory_gateways_id_seq OWNED BY directory_gateways.id;


--
-- Name: directory_gateways_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('directory_gateways_id_seq', 1, false);


--
-- Name: directory_global_params; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE directory_global_params (
    id integer NOT NULL,
    param_name character varying(64) NOT NULL,
    param_value character varying(128) NOT NULL,
    directory_id integer NOT NULL
);


ALTER TABLE public.directory_global_params OWNER TO freeswitch;

--
-- Name: directory_global_params_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE directory_global_params_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.directory_global_params_id_seq OWNER TO freeswitch;

--
-- Name: directory_global_params_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE directory_global_params_id_seq OWNED BY directory_global_params.id;


--
-- Name: directory_global_params_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('directory_global_params_id_seq', 1, false);


--
-- Name: directory_global_vars; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE directory_global_vars (
    id integer NOT NULL,
    var_name character varying(64) NOT NULL,
    var_value character varying(128) NOT NULL,
    directory_id integer NOT NULL
);


ALTER TABLE public.directory_global_vars OWNER TO freeswitch;

--
-- Name: directory_global_vars_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE directory_global_vars_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.directory_global_vars_id_seq OWNER TO freeswitch;

--
-- Name: directory_global_vars_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE directory_global_vars_id_seq OWNED BY directory_global_vars.id;


--
-- Name: directory_global_vars_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('directory_global_vars_id_seq', 1, false);


--
-- Name: directory_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE directory_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.directory_id_seq OWNER TO freeswitch;

--
-- Name: directory_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE directory_id_seq OWNED BY directory.id;


--
-- Name: directory_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('directory_id_seq', 1, false);


--
-- Name: directory_params; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE directory_params (
    id integer NOT NULL,
    directory_id integer NOT NULL,
    param_name character varying(256) NOT NULL,
    param_value character varying(256) NOT NULL
);


ALTER TABLE public.directory_params OWNER TO freeswitch;

--
-- Name: directory_params_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE directory_params_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.directory_params_id_seq OWNER TO freeswitch;

--
-- Name: directory_params_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE directory_params_id_seq OWNED BY directory_params.id;


--
-- Name: directory_params_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('directory_params_id_seq', 1, false);


--
-- Name: directory_vars; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE directory_vars (
    id integer NOT NULL,
    directory_id integer NOT NULL,
    var_name character varying(256) NOT NULL,
    var_value character varying(256) NOT NULL
);


ALTER TABLE public.directory_vars OWNER TO freeswitch;

--
-- Name: directory_vars_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE directory_vars_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.directory_vars_id_seq OWNER TO freeswitch;

--
-- Name: directory_vars_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE directory_vars_id_seq OWNED BY directory_vars.id;


--
-- Name: directory_vars_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('directory_vars_id_seq', 1, false);


--
-- Name: easyroute_conf; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE easyroute_conf (
    id integer NOT NULL,
    param_name character varying(256) NOT NULL,
    param_value character varying(256) NOT NULL
);


ALTER TABLE public.easyroute_conf OWNER TO freeswitch;

--
-- Name: easyroute_conf_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE easyroute_conf_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.easyroute_conf_id_seq OWNER TO freeswitch;

--
-- Name: easyroute_conf_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE easyroute_conf_id_seq OWNED BY easyroute_conf.id;


--
-- Name: easyroute_conf_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('easyroute_conf_id_seq', 1, false);


--
-- Name: easyroute_data; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE easyroute_data (
    id integer NOT NULL,
    gateway character varying(128) NOT NULL,
    "group" character varying(128) NOT NULL,
    call_limit character varying(16) NOT NULL,
    tech_prefix character varying(128) NOT NULL,
    acctcode character varying(128) NOT NULL,
    destination_number character varying(128) NOT NULL
);


ALTER TABLE public.easyroute_data OWNER TO freeswitch;

--
-- Name: easyroute_data_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE easyroute_data_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.easyroute_data_id_seq OWNER TO freeswitch;

--
-- Name: easyroute_data_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE easyroute_data_id_seq OWNED BY easyroute_data.id;


--
-- Name: easyroute_data_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('easyroute_data_id_seq', 1, false);


--
-- Name: iax_conf; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE iax_conf (
    id integer NOT NULL,
    profile_name character varying(256) NOT NULL
);


ALTER TABLE public.iax_conf OWNER TO freeswitch;

--
-- Name: iax_conf_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE iax_conf_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.iax_conf_id_seq OWNER TO freeswitch;

--
-- Name: iax_conf_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE iax_conf_id_seq OWNED BY iax_conf.id;


--
-- Name: iax_conf_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('iax_conf_id_seq', 1, false);


--
-- Name: iax_settings; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE iax_settings (
    id integer NOT NULL,
    iax_id integer NOT NULL,
    param_name character varying(256) NOT NULL,
    param_value character varying(256) NOT NULL
);


ALTER TABLE public.iax_settings OWNER TO freeswitch;

--
-- Name: iax_settings_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE iax_settings_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.iax_settings_id_seq OWNER TO freeswitch;

--
-- Name: iax_settings_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE iax_settings_id_seq OWNED BY iax_settings.id;


--
-- Name: iax_settings_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('iax_settings_id_seq', 1, false);


--
-- Name: ivr_conf; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE ivr_conf (
    id integer NOT NULL,
    name character varying(64) NOT NULL,
    greet_long character varying(256) NOT NULL,
    greet_short character varying(256) NOT NULL,
    invalid_sound character varying(256) NOT NULL,
    exit_sound character varying(256) NOT NULL,
    max_failures integer NOT NULL,
    timeout integer NOT NULL,
    tts_engine character varying(64) NOT NULL,
    tts_voice character varying(64) NOT NULL
);


ALTER TABLE public.ivr_conf OWNER TO freeswitch;

--
-- Name: ivr_conf_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE ivr_conf_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.ivr_conf_id_seq OWNER TO freeswitch;

--
-- Name: ivr_conf_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE ivr_conf_id_seq OWNED BY ivr_conf.id;


--
-- Name: ivr_conf_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('ivr_conf_id_seq', 1, false);


--
-- Name: ivr_entries; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE ivr_entries (
    id integer NOT NULL,
    ivr_id integer NOT NULL,
    action character varying(64) NOT NULL,
    digits character varying(64) NOT NULL,
    params character varying(256) NOT NULL
);


ALTER TABLE public.ivr_entries OWNER TO freeswitch;

--
-- Name: ivr_entries_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE ivr_entries_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.ivr_entries_id_seq OWNER TO freeswitch;

--
-- Name: ivr_entries_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE ivr_entries_id_seq OWNED BY ivr_entries.id;


--
-- Name: ivr_entries_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('ivr_entries_id_seq', 1, false);


--
-- Name: lcr; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE lcr (
    id integer NOT NULL,
    digits numeric(20,0),
    rate numeric(11,5) NOT NULL,
    intrastate_rate numeric(11,5) NOT NULL,
    intralata_rate numeric(11,5) NOT NULL,
    carrier_id integer NOT NULL,
    lead_strip integer DEFAULT 0 NOT NULL,
    trail_strip integer DEFAULT 0 NOT NULL,
    prefix character varying(16) DEFAULT ''::character varying NOT NULL,
    suffix character varying(16) DEFAULT ''::character varying NOT NULL,
    lcr_profile integer DEFAULT 0 NOT NULL,
    date_start timestamp with time zone DEFAULT '1970-01-01 00:00:00-05'::timestamp with time zone NOT NULL,
    date_end timestamp with time zone DEFAULT '2030-12-31 00:00:00-05'::timestamp with time zone NOT NULL,
    quality numeric(10,6) DEFAULT 0 NOT NULL,
    reliability numeric(10,6) DEFAULT 0 NOT NULL,
    cid character varying(32) DEFAULT ''::character varying NOT NULL,
    enabled boolean DEFAULT true NOT NULL
);


ALTER TABLE public.lcr OWNER TO freeswitch;

--
-- Name: lcr_conf; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE lcr_conf (
    id integer NOT NULL,
    param_name character varying(256) NOT NULL,
    param_value character varying(256) NOT NULL
);


ALTER TABLE public.lcr_conf OWNER TO freeswitch;

--
-- Name: lcr_conf_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE lcr_conf_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.lcr_conf_id_seq OWNER TO freeswitch;

--
-- Name: lcr_conf_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE lcr_conf_id_seq OWNED BY lcr_conf.id;


--
-- Name: lcr_conf_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('lcr_conf_id_seq', 1, true);


--
-- Name: lcr_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE lcr_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.lcr_id_seq OWNER TO freeswitch;

--
-- Name: lcr_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE lcr_id_seq OWNED BY lcr.id;


--
-- Name: lcr_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('lcr_id_seq', 1, false);


--
-- Name: lcr_profiles; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE lcr_profiles (
    id integer NOT NULL,
    profile_name character varying(128) NOT NULL
);


ALTER TABLE public.lcr_profiles OWNER TO freeswitch;

--
-- Name: lcr_profiles_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE lcr_profiles_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.lcr_profiles_id_seq OWNER TO freeswitch;

--
-- Name: lcr_profiles_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE lcr_profiles_id_seq OWNED BY lcr_profiles.id;


--
-- Name: lcr_profiles_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('lcr_profiles_id_seq', 3, true);


--
-- Name: lcr_settings; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE lcr_settings (
    id integer NOT NULL,
    lcr_id integer NOT NULL,
    param_name character varying(256) NOT NULL,
    param_value character varying(256) NOT NULL
);


ALTER TABLE public.lcr_settings OWNER TO freeswitch;

--
-- Name: lcr_settings_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE lcr_settings_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.lcr_settings_id_seq OWNER TO freeswitch;

--
-- Name: lcr_settings_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE lcr_settings_id_seq OWNED BY lcr_settings.id;


--
-- Name: lcr_settings_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('lcr_settings_id_seq', 1, true);


--
-- Name: limit_conf; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE limit_conf (
    id integer NOT NULL,
    name character varying(256) NOT NULL,
    value character varying(256) NOT NULL
);


ALTER TABLE public.limit_conf OWNER TO freeswitch;

--
-- Name: limit_conf_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE limit_conf_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.limit_conf_id_seq OWNER TO freeswitch;

--
-- Name: limit_conf_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE limit_conf_id_seq OWNED BY limit_conf.id;


--
-- Name: limit_conf_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('limit_conf_id_seq', 1, false);


--
-- Name: limit_data; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE limit_data (
    hostname character varying(255) DEFAULT NULL::character varying,
    realm character varying(255) DEFAULT NULL::character varying,
    id character varying(255) DEFAULT NULL::character varying,
    uuid character varying(255) DEFAULT NULL::character varying
);


ALTER TABLE public.limit_data OWNER TO freeswitch;

--
-- Name: local_stream_conf; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE local_stream_conf (
    id integer NOT NULL,
    directory_name character varying(256) NOT NULL,
    directory_path text NOT NULL,
    param_name character varying(256) NOT NULL,
    param_value character varying(256) NOT NULL
);


ALTER TABLE public.local_stream_conf OWNER TO freeswitch;

--
-- Name: local_stream_conf_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE local_stream_conf_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.local_stream_conf_id_seq OWNER TO freeswitch;

--
-- Name: local_stream_conf_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE local_stream_conf_id_seq OWNED BY local_stream_conf.id;


--
-- Name: local_stream_conf_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('local_stream_conf_id_seq', 1, false);


--
-- Name: modless_conf; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE modless_conf (
    id integer NOT NULL,
    conf_name character varying(64) NOT NULL
);


ALTER TABLE public.modless_conf OWNER TO freeswitch;

--
-- Name: modless_conf_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE modless_conf_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.modless_conf_id_seq OWNER TO freeswitch;

--
-- Name: modless_conf_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE modless_conf_id_seq OWNED BY modless_conf.id;


--
-- Name: modless_conf_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('modless_conf_id_seq', 1, false);


--
-- Name: npa_nxx_company_ocn; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE npa_nxx_company_ocn (
    npa smallint NOT NULL,
    nxx smallint NOT NULL,
    company_type text,
    ocn text,
    company_name text,
    lata integer,
    ratecenter text,
    state text
);


ALTER TABLE public.npa_nxx_company_ocn OWNER TO freeswitch;

--
-- Name: post_load_modules_conf; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE post_load_modules_conf (
    id integer NOT NULL,
    module_name character varying(64) NOT NULL,
    load_module boolean DEFAULT true NOT NULL,
    priority integer DEFAULT 1000 NOT NULL
);


ALTER TABLE public.post_load_modules_conf OWNER TO freeswitch;

--
-- Name: post_load_modules_conf_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE post_load_modules_conf_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.post_load_modules_conf_id_seq OWNER TO freeswitch;

--
-- Name: post_load_modules_conf_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE post_load_modules_conf_id_seq OWNED BY post_load_modules_conf.id;


--
-- Name: post_load_modules_conf_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('post_load_modules_conf_id_seq', 1, false);


--
-- Name: rss_conf; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE rss_conf (
    id integer NOT NULL,
    directory_id integer NOT NULL,
    feed text NOT NULL,
    local_file text NOT NULL,
    description text NOT NULL,
    priority integer DEFAULT 1000 NOT NULL
);


ALTER TABLE public.rss_conf OWNER TO freeswitch;

--
-- Name: rss_conf_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE rss_conf_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.rss_conf_id_seq OWNER TO freeswitch;

--
-- Name: rss_conf_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE rss_conf_id_seq OWNED BY rss_conf.id;


--
-- Name: rss_conf_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('rss_conf_id_seq', 1, false);


--
-- Name: sip_authentication; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE sip_authentication (
    nonce character varying(255),
    expires integer,
    profile_name character varying(255),
    hostname character varying(255)
);


ALTER TABLE public.sip_authentication OWNER TO freeswitch;

--
-- Name: sip_dialogs; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE sip_dialogs (
    call_id character varying(255),
    uuid character varying(255),
    sip_to_user character varying(255),
    sip_to_host character varying(255),
    sip_from_user character varying(255),
    sip_from_host character varying(255),
    contact_user character varying(255),
    contact_host character varying(255),
    state character varying(255),
    direction character varying(255),
    user_agent character varying(255),
    profile_name character varying(255),
    hostname character varying(255)
);


ALTER TABLE public.sip_dialogs OWNER TO freeswitch;

--
-- Name: sip_presence; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE sip_presence (
    sip_user character varying(255),
    sip_host character varying(255),
    status character varying(255),
    rpid character varying(255),
    expires integer,
    user_agent character varying(255),
    profile_name character varying(255),
    hostname character varying(255),
    network_ip character varying(255),
    network_port character varying(6)
);


ALTER TABLE public.sip_presence OWNER TO freeswitch;

--
-- Name: sip_registrations; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE sip_registrations (
    call_id character varying(255),
    sip_user character varying(255),
    sip_host character varying(255),
    presence_hosts character varying(255),
    contact character varying(1024),
    status character varying(255),
    rpid character varying(255),
    expires integer,
    user_agent character varying(255),
    server_user character varying(255),
    server_host character varying(255),
    profile_name character varying(255),
    hostname character varying(255),
    network_ip character varying(255),
    network_port character varying(6),
    sip_username character varying(255),
    sip_realm character varying(255),
    mwi_user character varying(255),
    mwi_host character varying(255)
);


ALTER TABLE public.sip_registrations OWNER TO freeswitch;

--
-- Name: sip_shared_appearance_dialogs; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE sip_shared_appearance_dialogs (
    profile_name character varying(255),
    hostname character varying(255),
    contact_str character varying(255),
    call_id character varying(255),
    network_ip character varying(255),
    expires integer
);


ALTER TABLE public.sip_shared_appearance_dialogs OWNER TO freeswitch;

--
-- Name: sip_shared_appearance_subscriptions; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE sip_shared_appearance_subscriptions (
    subscriber character varying(255),
    call_id character varying(255),
    aor character varying(255),
    profile_name character varying(255),
    hostname character varying(255),
    contact_str character varying(255),
    network_ip character varying(255)
);


ALTER TABLE public.sip_shared_appearance_subscriptions OWNER TO freeswitch;

--
-- Name: sip_subscriptions; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE sip_subscriptions (
    proto character varying(255),
    sip_user character varying(255),
    sip_host character varying(255),
    sub_to_user character varying(255),
    sub_to_host character varying(255),
    presence_hosts character varying(255),
    event character varying(255),
    contact character varying(1024),
    call_id character varying(255),
    full_from character varying(255),
    full_via character varying(255),
    expires integer,
    user_agent character varying(255),
    accept character varying(255),
    profile_name character varying(255),
    hostname character varying(255),
    network_port character varying(6),
    network_ip character varying(255)
);


ALTER TABLE public.sip_subscriptions OWNER TO freeswitch;

--
-- Name: sofia_aliases; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE sofia_aliases (
    id integer NOT NULL,
    sofia_id integer NOT NULL,
    alias_name character varying(256) NOT NULL
);


ALTER TABLE public.sofia_aliases OWNER TO freeswitch;

--
-- Name: sofia_aliases_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE sofia_aliases_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.sofia_aliases_id_seq OWNER TO freeswitch;

--
-- Name: sofia_aliases_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE sofia_aliases_id_seq OWNED BY sofia_aliases.id;


--
-- Name: sofia_aliases_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('sofia_aliases_id_seq', 1, false);


--
-- Name: sofia_conf; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE sofia_conf (
    id integer NOT NULL,
    profile_name character varying(256) NOT NULL
);


ALTER TABLE public.sofia_conf OWNER TO freeswitch;

--
-- Name: sofia_conf_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE sofia_conf_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.sofia_conf_id_seq OWNER TO freeswitch;

--
-- Name: sofia_conf_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE sofia_conf_id_seq OWNED BY sofia_conf.id;


--
-- Name: sofia_conf_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('sofia_conf_id_seq', 1, false);


--
-- Name: sofia_domains; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE sofia_domains (
    id integer NOT NULL,
    sofia_id integer NOT NULL,
    domain_name character varying(256) NOT NULL,
    parse boolean DEFAULT false NOT NULL
);


ALTER TABLE public.sofia_domains OWNER TO freeswitch;

--
-- Name: sofia_domains_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE sofia_domains_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.sofia_domains_id_seq OWNER TO freeswitch;

--
-- Name: sofia_domains_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE sofia_domains_id_seq OWNED BY sofia_domains.id;


--
-- Name: sofia_domains_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('sofia_domains_id_seq', 1, false);


--
-- Name: sofia_gateways; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE sofia_gateways (
    id integer NOT NULL,
    sofia_id integer NOT NULL,
    gateway_name character varying(256) NOT NULL,
    gateway_param character varying(256) NOT NULL,
    gateway_value character varying(256) NOT NULL
);


ALTER TABLE public.sofia_gateways OWNER TO freeswitch;

--
-- Name: sofia_gateways_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE sofia_gateways_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.sofia_gateways_id_seq OWNER TO freeswitch;

--
-- Name: sofia_gateways_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE sofia_gateways_id_seq OWNED BY sofia_gateways.id;


--
-- Name: sofia_gateways_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('sofia_gateways_id_seq', 1, false);


--
-- Name: sofia_settings; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE sofia_settings (
    id integer NOT NULL,
    sofia_id integer NOT NULL,
    param_name character varying(256) NOT NULL,
    param_value character varying(256) NOT NULL
);


ALTER TABLE public.sofia_settings OWNER TO freeswitch;

--
-- Name: sofia_settings_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE sofia_settings_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.sofia_settings_id_seq OWNER TO freeswitch;

--
-- Name: sofia_settings_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE sofia_settings_id_seq OWNED BY sofia_settings.id;


--
-- Name: sofia_settings_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('sofia_settings_id_seq', 1, false);


--
-- Name: voicemail_conf; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE voicemail_conf (
    id integer NOT NULL,
    vm_profile character varying(256) NOT NULL
);


ALTER TABLE public.voicemail_conf OWNER TO freeswitch;

--
-- Name: voicemail_conf_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE voicemail_conf_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.voicemail_conf_id_seq OWNER TO freeswitch;

--
-- Name: voicemail_conf_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE voicemail_conf_id_seq OWNED BY voicemail_conf.id;


--
-- Name: voicemail_conf_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('voicemail_conf_id_seq', 1, false);


--
-- Name: voicemail_email; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE voicemail_email (
    id integer NOT NULL,
    voicemail_id integer NOT NULL,
    param_name character varying(256) NOT NULL,
    param_value character varying(256) NOT NULL
);


ALTER TABLE public.voicemail_email OWNER TO freeswitch;

--
-- Name: voicemail_email_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE voicemail_email_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.voicemail_email_id_seq OWNER TO freeswitch;

--
-- Name: voicemail_email_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE voicemail_email_id_seq OWNED BY voicemail_email.id;


--
-- Name: voicemail_email_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('voicemail_email_id_seq', 1, false);


--
-- Name: voicemail_msgs; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE voicemail_msgs (
    created_epoch integer,
    read_epoch integer,
    username character varying(255),
    domain character varying(255),
    uuid character varying(255),
    cid_name character varying(255),
    cid_number character varying(255),
    in_folder character varying(255),
    file_path character varying(255),
    message_len integer,
    flags character varying(255),
    read_flags character varying(255)
);


ALTER TABLE public.voicemail_msgs OWNER TO freeswitch;

--
-- Name: voicemail_prefs; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE voicemail_prefs (
    username character varying(255),
    domain character varying(255),
    name_path character varying(255),
    greeting_path character varying(255),
    password character varying(255)
);


ALTER TABLE public.voicemail_prefs OWNER TO freeswitch;

--
-- Name: voicemail_settings; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE voicemail_settings (
    id integer NOT NULL,
    voicemail_id integer NOT NULL,
    param_name character varying(256) NOT NULL,
    param_value character varying(256) NOT NULL
);


ALTER TABLE public.voicemail_settings OWNER TO freeswitch;

--
-- Name: voicemail_settings_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE voicemail_settings_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.voicemail_settings_id_seq OWNER TO freeswitch;

--
-- Name: voicemail_settings_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE voicemail_settings_id_seq OWNED BY voicemail_settings.id;


--
-- Name: voicemail_settings_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('voicemail_settings_id_seq', 1, false);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE acl_lists ALTER COLUMN id SET DEFAULT nextval('acl_lists_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE acl_nodes ALTER COLUMN id SET DEFAULT nextval('acl_nodes_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE carrier_gateway ALTER COLUMN id SET DEFAULT nextval('carrier_gateway_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE carriers ALTER COLUMN id SET DEFAULT nextval('carriers_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE conference_advertise ALTER COLUMN id SET DEFAULT nextval('conference_advertise_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE conference_controls ALTER COLUMN id SET DEFAULT nextval('conference_controls_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE conference_profiles ALTER COLUMN id SET DEFAULT nextval('conference_profiles_id_seq'::regclass);


--
-- Name: dialplan_id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE dialplan ALTER COLUMN dialplan_id SET DEFAULT nextval('dialplan_dialplan_id_seq'::regclass);


--
-- Name: action_id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE dialplan_actions ALTER COLUMN action_id SET DEFAULT nextval('dialplan_actions_action_id_seq'::regclass);


--
-- Name: condition_id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE dialplan_condition ALTER COLUMN condition_id SET DEFAULT nextval('dialplan_condition_condition_id_seq'::regclass);


--
-- Name: context_id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE dialplan_context ALTER COLUMN context_id SET DEFAULT nextval('dialplan_context_context_id_seq'::regclass);


--
-- Name: extension_id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE dialplan_extension ALTER COLUMN extension_id SET DEFAULT nextval('dialplan_extension_extension_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE dialplan_special ALTER COLUMN id SET DEFAULT nextval('dialplan_special_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE dingaling_profile_params ALTER COLUMN id SET DEFAULT nextval('dingaling_profile_params_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE dingaling_profiles ALTER COLUMN id SET DEFAULT nextval('dingaling_profiles_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE dingaling_settings ALTER COLUMN id SET DEFAULT nextval('dingaling_settings_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE directory ALTER COLUMN id SET DEFAULT nextval('directory_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE directory_domains ALTER COLUMN id SET DEFAULT nextval('directory_domains_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE directory_gateway_params ALTER COLUMN id SET DEFAULT nextval('directory_gateway_params_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE directory_gateways ALTER COLUMN id SET DEFAULT nextval('directory_gateways_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE directory_global_params ALTER COLUMN id SET DEFAULT nextval('directory_global_params_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE directory_global_vars ALTER COLUMN id SET DEFAULT nextval('directory_global_vars_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE directory_params ALTER COLUMN id SET DEFAULT nextval('directory_params_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE directory_vars ALTER COLUMN id SET DEFAULT nextval('directory_vars_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE easyroute_conf ALTER COLUMN id SET DEFAULT nextval('easyroute_conf_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE easyroute_data ALTER COLUMN id SET DEFAULT nextval('easyroute_data_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE iax_conf ALTER COLUMN id SET DEFAULT nextval('iax_conf_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE iax_settings ALTER COLUMN id SET DEFAULT nextval('iax_settings_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE ivr_conf ALTER COLUMN id SET DEFAULT nextval('ivr_conf_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE ivr_entries ALTER COLUMN id SET DEFAULT nextval('ivr_entries_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE lcr ALTER COLUMN id SET DEFAULT nextval('lcr_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE lcr_conf ALTER COLUMN id SET DEFAULT nextval('lcr_conf_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE lcr_profiles ALTER COLUMN id SET DEFAULT nextval('lcr_profiles_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE lcr_settings ALTER COLUMN id SET DEFAULT nextval('lcr_settings_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE limit_conf ALTER COLUMN id SET DEFAULT nextval('limit_conf_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE local_stream_conf ALTER COLUMN id SET DEFAULT nextval('local_stream_conf_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE modless_conf ALTER COLUMN id SET DEFAULT nextval('modless_conf_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE post_load_modules_conf ALTER COLUMN id SET DEFAULT nextval('post_load_modules_conf_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE rss_conf ALTER COLUMN id SET DEFAULT nextval('rss_conf_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE sofia_aliases ALTER COLUMN id SET DEFAULT nextval('sofia_aliases_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE sofia_conf ALTER COLUMN id SET DEFAULT nextval('sofia_conf_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE sofia_domains ALTER COLUMN id SET DEFAULT nextval('sofia_domains_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE sofia_gateways ALTER COLUMN id SET DEFAULT nextval('sofia_gateways_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE sofia_settings ALTER COLUMN id SET DEFAULT nextval('sofia_settings_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE voicemail_conf ALTER COLUMN id SET DEFAULT nextval('voicemail_conf_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE voicemail_email ALTER COLUMN id SET DEFAULT nextval('voicemail_email_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: freeswitch
--

ALTER TABLE voicemail_settings ALTER COLUMN id SET DEFAULT nextval('voicemail_settings_id_seq'::regclass);


--
-- Data for Name: acl_lists; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY acl_lists (id, acl_name, default_policy) FROM stdin;
1	rfc1918	deny
2	lan	allow
3	default	allow
\.


--
-- Data for Name: acl_nodes; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY acl_nodes (id, cidr, type, list_id) FROM stdin;
1	192.168.0.0/16	allow	1
2	10.0.0.0/8	allow	1
3	172.16.0.0/12	allow	1
\.


--
-- Data for Name: carrier_gateway; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY carrier_gateway (id, carrier_id, prefix, suffix, codec, enabled) FROM stdin;
\.


--
-- Data for Name: carriers; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY carriers (id, carrier_name, enabled) FROM stdin;
\.


--
-- Data for Name: conference_advertise; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY conference_advertise (id, room, status) FROM stdin;
1	3000@$${domain}	Freeswitch Conference
2	3001@$${domain}	FreeSWITCH Conference 2
3	3002@$${domain}	FreeSWITCH Conference 3
\.


--
-- Data for Name: conference_controls; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY conference_controls (id, conf_group, action, digits) FROM stdin;
1	default	mute	0
2	default	deaf_mute	*
3	default	energy up	9
4	default	energy equ	8
5	default	energy dn	7
6	default	vol talk up	3
7	default	vol talk dn	1
8	default	vol talk zero	2
9	default	vol listen up	6
10	default	vol listen dn	4
11	default	vol listen zero	5
12	default	hangup	#
\.


--
-- Data for Name: conference_profiles; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY conference_profiles (id, profile_name, param_name, param_value) FROM stdin;
1	default	domain	$${domain}
2	default	rate	8000
3	default	interval	20
4	default	energy-level	300
5	default	moh-sound	$${moh_uri}
6	default	caller-id-name	$${outbound_caller_name}
7	default	caller-id-number	$${outbound_caller_number}
\.


--
-- Data for Name: dialplan; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY dialplan (dialplan_id, domain, ip_address) FROM stdin;
1	freeswitch	127.0.0.1
\.


--
-- Data for Name: dialplan_actions; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY dialplan_actions (action_id, condition_id, application, data, type, weight) FROM stdin;
1	2	deflect	${destination_number}	action	10
2	5	set	domain_name=$${domain}	action	20
3	5	set	domain_name=${sip_auth_realm}	anti-action	30
4	7	set	domain_name=$${domain}	action	40
5	9	set	open=true	action	50
6	10	answer		action	60
7	10	intercept	${db(select/${domain_name}-last_dial/global)}	action	70
8	10	sleep	2000	action	80
9	11	answer		action	90
10	11	intercept	${db(select/${domain_name}-last_dial/${callgroup})}	action	100
11	11	sleep	2000	action	110
12	12	answer		action	120
13	12	intercept	${db(select/${domain_name}-last_dial_ext/$1)}	action	130
14	12	sleep	2000	action	140
15	13	transfer	${db(select/${domain_name}-last_dial/${caller_id_number})}	action	150
16	14	set	use_profile=${cond(${acl($${local_ip_v4} rfc1918)} == true ? nat : default)}	action	160
17	14	set	use_profile=${cond(${acl(${network_addr} rfc1918)} == true ? nat : default)}	anti-action	170
18	15	set_user	default@${domain_name}	action	180
19	16	info		action	190
20	17	set	sip_secure_media=true	action	200
21	18	db	insert/${domain_name}-spymap/${caller_id_number}/${uuid}	action	210
22	18	db	insert/${domain_name}-last_dial/${caller_id_number}/${destination_number}	action	220
23	18	db	insert/${domain_name}-last_dial/global/${uuid}	action	230
24	19	eval	${snom_bind_key(2 off DND ${sip_from_user} ${sip_from_host} ${sofia_profile_name} message notused)}	action	240
25	19	transfer	3000	action	250
26	20	eval	${snom_bind_key(2 on DND ${sip_from_user} ${sip_from_host} ${sofia_profile_name} message api+uuid_transfer ${uuid} 9001)}	action	260
27	20	playback	$${hold_music}	action	270
28	21	answer		action	280
29	21	eavesdrop	${db(select/${domain_name}-spymap/$1)}	action	290
30	22	answer		action	300
31	22	set	eavesdrop_indicate_failed=tone_stream://%(500, 0, 320)	action	310
32	22	set	eavesdrop_indicate_new=tone_stream://%(500, 0, 620)	action	320
33	22	set	eavesdrop_indicate_idle=tone_stream://%(250, 0, 920)	action	330
34	22	eavesdrop	all	action	340
35	23	transfer	${db(select/${domain_name}-call_return/${caller_id_number})}	action	350
36	24	answer		action	360
37	24	group	delete:$1@${domain_name}:${sofia_contact(${sip_from_user}@${domain_name})}	action	370
38	24	gentones	%(1000, 0, 320)	action	380
39	25	answer		action	390
40	25	group	insert:$1@${domain_name}:${sofia_contact(${sip_from_user}@${domain_name})}	action	400
41	25	gentones	%(1000, 0, 640)	action	410
42	26	bridge	{ignore_early_media=true}${group(call:$1@${domain_name})}	action	420
43	27	set	call_timeout=10	action	430
44	27	bridge	{ignore_early_media=true}${group(call:$1@${domain_name}:order)}	action	440
45	28	set	dialed_extension=$1	action	450
46	28	export	sip_auto_answer=true	action	460
47	28	bridge	user/${dialed_extension}@${domain_name}	action	470
48	29	set	dialed_extension=$1	action	480
49	29	export	dialed_extension=$1	action	490
50	30	set	voicemail_authorized=${sip_authorized}	action	500
51	30	answer		action	510
52	30	sleep	1000	action	520
53	30	voicemail	check default ${domain_name} ${dialed_extension}	action	530
54	30	bind_meta_app	1 b s execute_extension::dx XML features	anti-action	540
55	30	bind_meta_app	2 b s record_session::$${base_dir}/recordings/${caller_id_number}.${strftime(%Y-%m-%d-%H-%M-%S)}.wav	anti-action	550
56	30	bind_meta_app	3 b s execute_extension::cf XML features	anti-action	560
57	30	set	ringback=${us-ring}	anti-action	570
58	30	set	transfer_ringback=$${hold_music}	anti-action	580
59	30	set	call_timeout=30	anti-action	590
60	30	set	hangup_after_bridge=true	anti-action	600
61	30	set	continue_on_fail=true	anti-action	610
62	30	db	insert/${domain_name}-call_return/${dialed_extension}/${caller_id_number}	anti-action	620
63	30	db	insert/${domain_name}-last_dial_ext/${dialed_extension}/${uuid}	anti-action	630
64	30	set	called_party_callgroup=${user_data(${dialed_extension}@${domain_name} var callgroup)}	anti-action	640
65	30	db	insert/${domain_name}-last_dial/${called_party_callgroup}/${uuid}	anti-action	650
66	30	bridge	user/${dialed_extension}@${domain_name}	anti-action	660
67	30	answer		anti-action	670
68	30	sleep	1000	anti-action	680
69	30	voicemail	default ${domain_name} ${dialed_extension}	anti-action	690
70	31	bridge	${group_call(sales@${domain_name})}	action	700
71	32	bridge	group/support@${domain_name}	action	710
72	33	bridge	group/billing@${domain_name}	action	720
73	34	set	transfer_ringback=$${hold_music}	action	730
74	34	transfer	1000 XML features	action	740
75	35	answer		action	750
76	35	sleep	1000	action	760
77	35	voicemail	check default ${domain_name}	action	770
78	36	bridge	sofia/${use_profile}/$1	action	780
79	37	answer		action	790
80	37	conference	$1-${domain_name}@default	action	800
81	38	answer		action	810
82	38	conference	$1-${domain_name}@wideband	action	820
83	39	answer		action	830
84	39	conference	$1-${domain_name}@ultrawideband	action	840
85	40	answer		action	850
86	40	conference	$1-${domain_name}@cdquality	action	860
87	41	bridge	sofia/${use_profile}/$1@conference.freeswitch.org	action	870
88	42	set	conference_auto_outcall_caller_id_name=Mad Boss1	action	880
89	42	set	conference_auto_outcall_caller_id_number=0911	action	890
90	42	set	conference_auto_outcall_timeout=60	action	900
91	42	set	conference_auto_outcall_flags=mute	action	910
92	42	set	conference_auto_outcall_prefix={sip_auto_answer=true,execute_on_answer='bind_meta_app 2 a s1 intercept::${uuid}'}	action	920
93	42	set	sip_exclude_contact=${network_addr}	action	930
94	42	conference_set_auto_outcall	${group_call(sales)}	action	940
95	42	conference	madboss_intercom1@default+flags{endconf|deaf}	action	950
96	43	set	conference_auto_outcall_caller_id_name=Mad Boss2	action	960
97	43	set	conference_auto_outcall_caller_id_number=0912	action	970
98	43	set	conference_auto_outcall_timeout=60	action	980
99	43	set	conference_auto_outcall_flags=mute	action	990
100	43	set	conference_auto_outcall_prefix={sip_auto_answer=true,execute_on_answer='bind_meta_app 2 a s1 intercept::${uuid}'}	action	1000
101	43	set	sip_exclude_contact=${network_addr}	action	1010
102	43	conference_set_auto_outcall	loopback/9999	action	1020
103	43	conference	madboss_intercom2@default+flags{endconf|deaf}	action	1030
104	44	set	conference_auto_outcall_caller_id_name=Mad Boss	action	1040
105	44	set	conference_auto_outcall_caller_id_number=0911	action	1050
106	44	set	conference_auto_outcall_timeout=60	action	1060
107	44	set	conference_auto_outcall_flags=none	action	1070
108	44	conference_set_auto_outcall	loopback/9999	action	1080
109	44	conference	madboss3@default	action	1090
110	45	answer		action	1100
111	45	sleep	2000	action	1110
112	45	ivr	demo_ivr	action	1120
113	46	conference	bridge:mydynaconf:sofia/${use_profile}/1234@conference.freeswitch.org	action	1130
114	47	answer		action	1140
115	47	esf_page_group		action	1150
116	48	set	fifo_music=$${hold_music}	action	1160
117	48	fifo	5900@${domain_name} in	action	1170
118	49	answer		action	1180
119	49	fifo	5900@${domain_name} out nowait	action	1190
120	51	fifo	$1@${domain_name} in undef $${hold_music}	action	1200
121	54	answer		action	1210
122	54	fifo	$1@${domain_name} out nowait	action	1220
123	57			expression	1230
124	57	fifo	$1@${domain_name} in undef $${hold_music}	action	1240
125	60	answer		action	1250
126	60	fifo	$1@${domain_name} out nowait	action	1260
127	61	pre_answer		action	1270
128	61	sleep	20000	action	1280
129	61	answer		action	1290
130	61	sleep	1000	action	1300
131	61	playback	voicemail/vm-goodbye.wav	action	1310
132	61	hangup		action	1320
133	62	ring_ready		action	1330
134	62	sleep	20000	action	1340
135	62	answer		action	1350
136	62	sleep	1000	action	1360
137	62	playback	voicemail/vm-goodbye.wav	action	1370
138	62	hangup		action	1380
139	63	set	ringback=$${uk-ring}	action	1390
140	63	bridge	loopback/wait	action	1400
141	64	set	ringback=$${hold_music}	action	1410
142	64	bridge	loopback/wait	action	1420
143	65	set	transfer_ringback=$${uk-ring}	action	1430
144	65	answer		action	1440
145	65	bridge	loopback/wait	action	1450
146	66	set	transfer_ringback=$${hold_music}	action	1460
147	66	answer		action	1470
148	66	bridge	loopback/wait	action	1480
149	67	answer		action	1490
150	67	info		action	1500
151	67	sleep	250	action	1510
152	67	hangup		action	1520
153	68	answer		action	1530
154	68	record_fsv	/tmp/testrecord.fsv	action	1540
155	69	answer		action	1550
156	69	play_fsv	/tmp/testrecord.fsv	action	1560
157	70	answer		action	1570
158	70	delay_echo	5000	action	1580
159	71	answer		action	1590
160	71	echo		action	1600
161	72	answer		action	1610
162	72	playback	tone_stream://%(10000,0,1004);loops=-1	action	1620
163	73	answer		action	1630
164	73	playback	tone_stream://path=${base_dir}/conf/tetris.ttml;loops=10	action	1640
165	75	answer		action	1650
166	75	execute_extension	is_secure XML features	action	1660
167	75	playback	$${hold_music}	action	1670
168	75	answer		anti-action	1680
169	75	playback	$${hold_music}	anti-action	1690
\.


--
-- Data for Name: dialplan_condition; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY dialplan_condition (condition_id, extension_id, field, expression, weight) FROM stdin;
1	1	$${unroll_loops}	^true$	10
2	1	${sip_looped_call}	^true$	20
3	2	${domain_name}	^$	30
4	2	source	mod_sofia	40
5	2	${sip_auth_realm}	^$	50
6	3	${domain_name}	^$	60
7	3	source	mod_openzap	70
8	4	${strftime(%w)}	^([1-5])$	80
9	4	${strftime(%H%M)}	^((09|1[0-7])[0-5][0-9]|1800)$	90
10	5	destination_number	^886$	100
11	6	destination_number	^*8$	110
12	7	destination_number	^**(d+)$	120
13	8	destination_number	^870$	130
14	9	${network_addr}	^$	140
15	9	${numbering_plan}	^$	150
16	9	${call_debug}	^true$	160
17	9	${sip_has_crypto}	^(AES_CM_128_HMAC_SHA1_32|AES_CM_128_HMAC_SHA1_80)$	170
18	9			180
19	10	destination_number	^9001$	190
20	11	destination_number	^9000$	200
21	12	destination_number	^88(.*)$|^*0(.*)$	210
22	13	destination_number	^779$	220
23	14	destination_number	^*69$|^869$|^lcr$	230
24	15	destination_number	^80(d{2})$	240
25	16	destination_number	^81(d{2})$	250
26	17	destination_number	^82(d{2})$	260
27	18	destination_number	^83(d{2})$	270
28	19	destination_number	^8(10[01][0-9])$	280
29	20	destination_number	^(20[01][0-9])$	290
30	20	destination_number	^${caller_id_number}$	300
31	21	destination_number	^3000$	310
32	22	destination_number	^3001$	320
33	23	destination_number	^3002$	330
34	24	destination_number	^operator$|^0$	340
35	25	destination_number	^vmain|4000$	350
36	26	destination_number	^sip:(.*)$	360
37	27	destination_number	^(30d{2})$	370
38	28	destination_number	^(31d{2})$	380
39	29	destination_number	^(32d{2})$	390
40	30	destination_number	^(33d{2})$	400
41	31	destination_number	^9(888|1616|3232)$	410
42	32	destination_number	^0911$	420
43	33	destination_number	^0912$	430
44	34	destination_number	^0913$	440
45	35	destination_number	^5000$	450
46	36	destination_number	^5001$	460
47	37	destination_number	^pagegroup$|^7243	470
48	38	destination_number	^5900$	480
49	39	destination_number	^5901$	490
50	40	source	mod_sofia	500
51	40	destination_number	park+(d+)	510
52	41	source	mod_sofia	520
53	41	destination_number	^parking$	530
54	41	${sip_to_params}	fifo=(d+)	540
55	42	source	mod_sofia	550
56	42	destination_number	callpark	560
57	42	${sip_refer_to}		570
58	43	source	mod_sofia	580
59	43	destination_number	pickup	590
60	43	${sip_to_params}	orbit=(d+)	600
61	44	destination_number	^wait$	610
62	45	destination_number	^9980$	620
63	46	destination_number	^9981$	630
64	47	destination_number	^9982$	640
65	48	destination_number	^9983$	650
66	49	destination_number	^9984$	660
67	50	destination_number	^9992$	670
68	51	destination_number	^9993$	680
69	52	destination_number	^9994$	690
70	53	destination_number	^9995$	700
71	54	destination_number	^9996$	710
72	55	destination_number	^9997$	720
73	56	destination_number	^9998$	730
74	57	destination_number	^9999$	740
75	57	${sip_has_crypto}	^(AES_CM_128_HMAC_SHA1_32|AES_CM_128_HMAC_SHA1_80)$	750
\.


--
-- Data for Name: dialplan_context; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY dialplan_context (context_id, dialplan_id, context, weight) FROM stdin;
1	1	default	10
2	1	public	20
\.


--
-- Data for Name: dialplan_extension; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY dialplan_extension (extension_id, context_id, name, continue, weight) FROM stdin;
1	1	unloop		10
2	1	set_domain	true	20
3	1	set_domain_openzap	true	30
4	1	tod_example	true	40
5	1	global-intercept		50
6	1	group-intercept		60
7	1	intercept-ext		70
8	1	redial		80
9	1	global	true	90
10	1	snom-demo-2		100
11	1	snom-demo-1		110
12	1	eavesdrop		120
13	1	eavesdrop		130
14	1	call_return		140
15	1	del-group		150
16	1	add-group		160
17	1	call-group-simo		170
18	1	call-group-order		180
19	1	extension-intercom		190
20	1	Local_Extension		200
21	1	group_dial_sales		210
22	1	group_dial_support		220
23	1	group_dial_billing		230
24	1	operator		240
25	1	vmain		250
26	1	sip_uri		260
27	1	nb_conferences		270
28	1	wb_conferences		280
29	1	uwb_conferences		290
30	1	cdquality_conferences		300
31	1	freeswitch_public_conf_via_sip		310
32	1	mad_boss_intercom		320
33	1	mad_boss_intercom		330
34	1	mad_boss		340
35	1	ivr_demo		350
36	1	dyanmic conference		360
37	1	rtp_multicast_page		370
38	1	park		380
39	1	unpark		390
40	1	park		400
41	1	unpark		410
42	1	park		420
43	1	unpark		430
44	1	wait		440
45	1	ringback_180		450
46	1	ringback_183_uk_ring		460
47	1	ringback_183_music_ring		470
48	1	ringback_post_answer_uk_ring		480
49	1	ringback_post_answer_music		490
50	1	show_info		500
51	1	video_record		510
52	1	video_playback		520
53	1	delay_echo		530
54	1	echo		540
55	1	milliwatt		550
56	2	tone_stream		560
57	2	hold_music		570
\.


--
-- Data for Name: dialplan_special; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY dialplan_special (id, context, class_file) FROM stdin;
\.


--
-- Data for Name: dingaling_profile_params; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY dingaling_profile_params (id, dingaling_id, param_name, param_value) FROM stdin;
1	1	password	secret
2	1	dialplan	XML,enum
3	1	server	example.org
4	1	name	fs.example.org
\.


--
-- Data for Name: dingaling_profiles; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY dingaling_profiles (id, profile_name, type) FROM stdin;
1	fs.intralanman.servehttp.com	component
\.


--
-- Data for Name: dingaling_settings; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY dingaling_settings (id, param_name, param_value) FROM stdin;
1	debug	0
2	codec-prefs	$${global_codec_prefs}
\.


--
-- Data for Name: directory; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY directory (id, username, domain) FROM stdin;
1	1000	example.com
2	1001	example.org
3	1002	example.net
5	1003	example.info
6	1004	example.com
7	1005	example.org
8	1006	example.net
9	1007	example.info
10	2000	default
11	1009	$${local_ip_v4}
\.


--
-- Data for Name: directory_domains; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY directory_domains (id, domain_name) FROM stdin;
1	freeswitch.org
2	sofaswitch.org
\.


--
-- Data for Name: directory_gateway_params; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY directory_gateway_params (id, d_gw_id, param_name, param_value) FROM stdin;
\.


--
-- Data for Name: directory_gateways; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY directory_gateways (id, directory_id, gateway_name) FROM stdin;
\.


--
-- Data for Name: directory_global_params; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY directory_global_params (id, param_name, param_value, directory_id) FROM stdin;
\.


--
-- Data for Name: directory_global_vars; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY directory_global_vars (id, var_name, var_value, directory_id) FROM stdin;
\.


--
-- Data for Name: directory_params; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY directory_params (id, directory_id, param_name, param_value) FROM stdin;
1	1	password	1234
2	1	vm-password	861000
3	2	password	1234
4	2	vm-password	861001
7	5	password	1234
8	6	password	1234
9	7	password	1234
10	8	password	123456
11	9	password	1234
12	10	password	123456
13	11	password	1234
14	3	vm-password	861002
15	3	password	1234
16	11	vm-password	861009
17	10	vm-password	1234
18	9	vm-password	861007
19	8	vm-password	861006
20	7	vm-password	861005
21	6	vm-password	861004
22	5	vm-password	861003
\.


--
-- Data for Name: directory_vars; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY directory_vars (id, directory_id, var_name, var_value) FROM stdin;
1	1	numbering_plan	US
2	2	numbering_plan	US
3	3	numbering_plan	AU
4	5	numbering_plan	US
5	5	area_code	434
\.


--
-- Data for Name: easyroute_conf; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY easyroute_conf (id, param_name, param_value) FROM stdin;
1	custom-query	SELECT * FROM easyroute_data WHERE number = '%s';
2	db-dsn	pgodbc
3	db-username	freeswitch
4	db-password	Fr33Sw1tch
5	default-techprofile	sofia
6	default-gateway	$${local_ip_v4}:5080
\.


--
-- Data for Name: easyroute_data; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY easyroute_data (id, gateway, "group", call_limit, tech_prefix, acctcode, destination_number) FROM stdin;
\.


--
-- Data for Name: iax_conf; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY iax_conf (id, profile_name) FROM stdin;
3	test_profile
\.


--
-- Data for Name: iax_settings; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY iax_settings (id, iax_id, param_name, param_value) FROM stdin;
35	3	debug	1
36	3	ip	$${local_ip_v4}
37	3	port	4569
38	3	context	public
39	3	dialplan	enum,XML
40	3	codec-prefs	$${global_codec_prefs}
41	3	codec-master	us
42	3	codec-rate	8
\.


--
-- Data for Name: ivr_conf; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY ivr_conf (id, name, greet_long, greet_short, invalid_sound, exit_sound, max_failures, timeout, tts_engine, tts_voice) FROM stdin;
\.


--
-- Data for Name: ivr_entries; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY ivr_entries (id, ivr_id, action, digits, params) FROM stdin;
\.


--
-- Data for Name: lcr; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY lcr (id, digits, rate, intrastate_rate, intralata_rate, carrier_id, lead_strip, trail_strip, prefix, suffix, lcr_profile, date_start, date_end, quality, reliability, cid, enabled) FROM stdin;
\.


--
-- Data for Name: lcr_conf; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY lcr_conf (id, param_name, param_value) FROM stdin;
1	odbc-dsn	pgodbc:freeswitch:Fr33Sw1tch
\.


--
-- Data for Name: lcr_profiles; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY lcr_profiles (id, profile_name) FROM stdin;
1	default
2	test
3	test2
\.


--
-- Data for Name: lcr_settings; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY lcr_settings (id, lcr_id, param_name, param_value) FROM stdin;
1	1	order_by	rate,quality,reliability
\.


--
-- Data for Name: limit_conf; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY limit_conf (id, name, value) FROM stdin;
\.


--
-- Data for Name: limit_data; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY limit_data (hostname, realm, id, uuid) FROM stdin;
\.


--
-- Data for Name: local_stream_conf; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY local_stream_conf (id, directory_name, directory_path, param_name, param_value) FROM stdin;
\.


--
-- Data for Name: modless_conf; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY modless_conf (id, conf_name) FROM stdin;
1	acl.conf
2	postl_load_switch.conf
3	post_load_modules.conf
\.


--
-- Data for Name: npa_nxx_company_ocn; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY npa_nxx_company_ocn (npa, nxx, company_type, ocn, company_name, lata, ratecenter, state) FROM stdin;
\.


--
-- Data for Name: post_load_modules_conf; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY post_load_modules_conf (id, module_name, load_module, priority) FROM stdin;
1	mod_sofia	t	2000
2	mod_iax	t	2000
3	mod_xml_rpc	t	100
4	mod_portaudio	t	1000
5	mod_enum	t	2000
6	mod_xml_cdr	t	1000
7	mod_spidermonkey	t	1000
8	mod_alsa	f	1000
10	mod_commands	t	1000
11	mod_voicemail	t	1000
12	mod_dialplan_xml	t	150
13	mod_dialplan_asterisk	t	150
14	mod_openzap	f	1000
15	mod_woomera	f	1000
17	mod_speex	t	500
18	mod_ilbc	f	1000
20	mod_g723_1	t	500
21	mod_g729	t	500
25	mod_amr	t	500
26	mod_fifo	t	1000
27	mod_limit	t	1000
28	mod_syslog	t	0
29	mod_dingaling	t	2000
30	mod_cdr_csv	t	1000
31	mod_event_socket	t	100
32	mod_multicast	f	1000
33	mod_zeroconf	f	1000
34	mod_xmpp_event	f	1000
35	mod_sndfile	t	1000
36	mod_native_file	t	1000
38	mod_local_stream	t	1000
39	mod_perl	f	1000
40	mod_python	f	1000
41	mod_java	f	1000
42	mod_cepstral	f	1000
43	mod_openmrcp	f	1000
44	mod_lumenvox	f	1000
45	mod_rss	f	1000
47	mod_say_fr	f	1000
48	mod_say_en	t	1000
49	mod_conference	t	1000
50	mod_ivr	f	1000
51	mod_console	t	0
52	mod_dptools	t	1500
53	mod_voipcodecs	t	500
55	mod_celt	t	500
37	mod_shout	f	1000
46	mod_say_de	f	1000
56	mod_say_es	t	1000
57	mod_lcr	t	1500
58	mod_easyroute	t	1500
59	mod_nibblebill	t	1500
54	mod_siren	t	500
9	mod_log_file	f	0
\.


--
-- Data for Name: rss_conf; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY rss_conf (id, directory_id, feed, local_file, description, priority) FROM stdin;
\.


--
-- Data for Name: sip_authentication; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY sip_authentication (nonce, expires, profile_name, hostname) FROM stdin;
\.


--
-- Data for Name: sip_dialogs; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY sip_dialogs (call_id, uuid, sip_to_user, sip_to_host, sip_from_user, sip_from_host, contact_user, contact_host, state, direction, user_agent, profile_name, hostname) FROM stdin;
\.


--
-- Data for Name: sip_presence; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY sip_presence (sip_user, sip_host, status, rpid, expires, user_agent, profile_name, hostname, network_ip, network_port) FROM stdin;
\.


--
-- Data for Name: sip_registrations; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY sip_registrations (call_id, sip_user, sip_host, presence_hosts, contact, status, rpid, expires, user_agent, server_user, server_host, profile_name, hostname, network_ip, network_port, sip_username, sip_realm, mwi_user, mwi_host) FROM stdin;
\.


--
-- Data for Name: sip_shared_appearance_dialogs; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY sip_shared_appearance_dialogs (profile_name, hostname, contact_str, call_id, network_ip, expires) FROM stdin;
\.


--
-- Data for Name: sip_shared_appearance_subscriptions; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY sip_shared_appearance_subscriptions (subscriber, call_id, aor, profile_name, hostname, contact_str, network_ip) FROM stdin;
\.


--
-- Data for Name: sip_subscriptions; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY sip_subscriptions (proto, sip_user, sip_host, sub_to_user, sub_to_host, presence_hosts, event, contact, call_id, full_from, full_via, expires, user_agent, accept, profile_name, hostname, network_port, network_ip) FROM stdin;
\.


--
-- Data for Name: sofia_aliases; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY sofia_aliases (id, sofia_id, alias_name) FROM stdin;
\.


--
-- Data for Name: sofia_conf; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY sofia_conf (id, profile_name) FROM stdin;
1	$${domain}
\.


--
-- Data for Name: sofia_domains; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY sofia_domains (id, sofia_id, domain_name, parse) FROM stdin;
\.


--
-- Data for Name: sofia_gateways; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY sofia_gateways (id, sofia_id, gateway_name, gateway_param, gateway_value) FROM stdin;
8	1	default	proxy	asterlink.com
9	1	default	realm	asterlink.com
10	1	default	username	USERNAME_HERE
11	1	default	register	false
12	1	default	expire-seconds	60
14	1	default	password	PASSWORD_HERE
13	1	default	retry_seconds	30
\.


--
-- Data for Name: sofia_settings; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY sofia_settings (id, sofia_id, param_name, param_value) FROM stdin;
2	1	auth-calls	true
5	1	debug	1
6	1	rfc2833-pt	101
7	1	sip-port	5060
8	1	dialplan	XML
10	1	codec-prefs	$${global_codec_prefs}
11	1	rtp-timeout-sec	300
12	1	rtp-ip	$${local_ip_v4}
13	1	sip-ip	$${local_ip_v4}
14	1	context	default
15	1	manage-presence	true
18	1	rtp-rewrite-timestampes	true
19	1	nonce-ttl	60
16	1	force-register-domain	$${domain}
36	1	odbc-dsn	pgodbc:freeswitch:Fr33Sw1tch
1	1	user-agent-string	FreeSWITCH/fs_curl
\.


--
-- Data for Name: voicemail_conf; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY voicemail_conf (id, vm_profile) FROM stdin;
1	default
\.


--
-- Data for Name: voicemail_email; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY voicemail_email (id, voicemail_id, param_name, param_value) FROM stdin;
1	1	template-file	voicemail.tpl
2	1	date-fmt	%A, %B %d %Y, %I %M %p
3	1	email-from	${voicemail_account}@${voicemail_domain}
\.


--
-- Data for Name: voicemail_msgs; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY voicemail_msgs (created_epoch, read_epoch, username, domain, uuid, cid_name, cid_number, in_folder, file_path, message_len, flags, read_flags) FROM stdin;
\.


--
-- Data for Name: voicemail_prefs; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY voicemail_prefs (username, domain, name_path, greeting_path, password) FROM stdin;
\.


--
-- Data for Name: voicemail_settings; Type: TABLE DATA; Schema: public; Owner: freeswitch
--

COPY voicemail_settings (id, voicemail_id, param_name, param_value) FROM stdin;
1	1	file-extension	wav
2	1	terminator-key	#
3	1	max-login-attempts	3
4	1	digit-timeout	10000
6	1	tone-spec	%(1000, 0, 640)
7	1	callback-dialplan	XML
8	1	callback-context	default
9	1	play-new-messages-key	1
10	1	play-saved-messages-key	2
11	1	main-menu-key	*
12	1	config-menu-key	5
13	1	record-greeting-key	1
14	1	choose-greeting-key	2
15	1	record-file-key	3
16	1	listen-file-key	1
17	1	record-name-key	3
18	1	save-file-key	9
19	1	delete-file-key	7
20	1	undelete-file-key	8
21	1	email-key	4
22	1	pause-key	0
23	1	restart-key	1
24	1	ff-key	6
25	1	rew-key	4
26	1	record-silence-threshold	200
27	1	record-silence-hits	2
28	1	web-template-file	web-vm.tpl
29	1	operator-extension	operator XML default
30	1	operator-key	9
31	1	odbc-dsn	pgodbc:freeswitch:Fr33Sw1tch
\.


--
-- Name: acl_lists_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY acl_lists
    ADD CONSTRAINT acl_lists_pkey PRIMARY KEY (id);


--
-- Name: acl_nodes_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY acl_nodes
    ADD CONSTRAINT acl_nodes_pkey PRIMARY KEY (id);


--
-- Name: carrier_gateway_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY carrier_gateway
    ADD CONSTRAINT carrier_gateway_pkey PRIMARY KEY (id);


--
-- Name: carriers_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY carriers
    ADD CONSTRAINT carriers_pkey PRIMARY KEY (id);


--
-- Name: conference_advertise_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY conference_advertise
    ADD CONSTRAINT conference_advertise_pkey PRIMARY KEY (id);


--
-- Name: conference_controls_conf_group_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY conference_controls
    ADD CONSTRAINT conference_controls_conf_group_key UNIQUE (conf_group, action);


--
-- Name: conference_controls_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY conference_controls
    ADD CONSTRAINT conference_controls_pkey PRIMARY KEY (id);


--
-- Name: conference_profiles_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY conference_profiles
    ADD CONSTRAINT conference_profiles_pkey PRIMARY KEY (id);


--
-- Name: conference_profiles_profile_name_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY conference_profiles
    ADD CONSTRAINT conference_profiles_profile_name_key UNIQUE (profile_name, param_name);


--
-- Name: dialplan_actions_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY dialplan_actions
    ADD CONSTRAINT dialplan_actions_pkey PRIMARY KEY (action_id);


--
-- Name: dialplan_condition_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY dialplan_condition
    ADD CONSTRAINT dialplan_condition_pkey PRIMARY KEY (condition_id);


--
-- Name: dialplan_context_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY dialplan_context
    ADD CONSTRAINT dialplan_context_pkey PRIMARY KEY (context_id);


--
-- Name: dialplan_extension_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY dialplan_extension
    ADD CONSTRAINT dialplan_extension_pkey PRIMARY KEY (extension_id);


--
-- Name: dialplan_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY dialplan
    ADD CONSTRAINT dialplan_pkey PRIMARY KEY (dialplan_id);


--
-- Name: dialplan_special_context_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY dialplan_special
    ADD CONSTRAINT dialplan_special_context_key UNIQUE (context);


--
-- Name: dialplan_special_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY dialplan_special
    ADD CONSTRAINT dialplan_special_pkey PRIMARY KEY (id);


--
-- Name: dingaling_profile_params_dingaling_id_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY dingaling_profile_params
    ADD CONSTRAINT dingaling_profile_params_dingaling_id_key UNIQUE (dingaling_id, param_name);


--
-- Name: dingaling_profile_params_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY dingaling_profile_params
    ADD CONSTRAINT dingaling_profile_params_pkey PRIMARY KEY (id);


--
-- Name: dingaling_profiles_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY dingaling_profiles
    ADD CONSTRAINT dingaling_profiles_pkey PRIMARY KEY (id);


--
-- Name: dingaling_profiles_profile_name_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY dingaling_profiles
    ADD CONSTRAINT dingaling_profiles_profile_name_key UNIQUE (profile_name);


--
-- Name: dingaling_settings_param_name_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY dingaling_settings
    ADD CONSTRAINT dingaling_settings_param_name_key UNIQUE (param_name);


--
-- Name: dingaling_settings_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY dingaling_settings
    ADD CONSTRAINT dingaling_settings_pkey PRIMARY KEY (id);


--
-- Name: directory_domains_domain_name_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY directory_domains
    ADD CONSTRAINT directory_domains_domain_name_key UNIQUE (domain_name);


--
-- Name: directory_domains_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY directory_domains
    ADD CONSTRAINT directory_domains_pkey PRIMARY KEY (id);


--
-- Name: directory_gateway_params_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY directory_gateway_params
    ADD CONSTRAINT directory_gateway_params_pkey PRIMARY KEY (id);


--
-- Name: directory_gateways_gateway_name_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY directory_gateways
    ADD CONSTRAINT directory_gateways_gateway_name_key UNIQUE (gateway_name);


--
-- Name: directory_gateways_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY directory_gateways
    ADD CONSTRAINT directory_gateways_pkey PRIMARY KEY (id);


--
-- Name: directory_global_params_directory_id_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY directory_global_params
    ADD CONSTRAINT directory_global_params_directory_id_key UNIQUE (directory_id, param_name);


--
-- Name: directory_global_params_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY directory_global_params
    ADD CONSTRAINT directory_global_params_pkey PRIMARY KEY (id);


--
-- Name: directory_global_vars_directory_id_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY directory_global_vars
    ADD CONSTRAINT directory_global_vars_directory_id_key UNIQUE (directory_id, var_name);


--
-- Name: directory_global_vars_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY directory_global_vars
    ADD CONSTRAINT directory_global_vars_pkey PRIMARY KEY (id);


--
-- Name: directory_params_directory_id_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY directory_params
    ADD CONSTRAINT directory_params_directory_id_key UNIQUE (directory_id, param_name);


--
-- Name: directory_params_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY directory_params
    ADD CONSTRAINT directory_params_pkey PRIMARY KEY (id);


--
-- Name: directory_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY directory
    ADD CONSTRAINT directory_pkey PRIMARY KEY (id);


--
-- Name: directory_username_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY directory
    ADD CONSTRAINT directory_username_key UNIQUE (username, domain);


--
-- Name: directory_vars_directory_id_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY directory_vars
    ADD CONSTRAINT directory_vars_directory_id_key UNIQUE (directory_id, var_name);


--
-- Name: directory_vars_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY directory_vars
    ADD CONSTRAINT directory_vars_pkey PRIMARY KEY (id);


--
-- Name: easyroute_conf_param_name_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY easyroute_conf
    ADD CONSTRAINT easyroute_conf_param_name_key UNIQUE (param_name);


--
-- Name: easyroute_conf_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY easyroute_conf
    ADD CONSTRAINT easyroute_conf_pkey PRIMARY KEY (id);


--
-- Name: easyroute_data_destination_number_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY easyroute_data
    ADD CONSTRAINT easyroute_data_destination_number_key UNIQUE (destination_number);


--
-- Name: easyroute_data_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY easyroute_data
    ADD CONSTRAINT easyroute_data_pkey PRIMARY KEY (id);


--
-- Name: iax_conf_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY iax_conf
    ADD CONSTRAINT iax_conf_pkey PRIMARY KEY (id);


--
-- Name: iax_conf_profile_name_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY iax_conf
    ADD CONSTRAINT iax_conf_profile_name_key UNIQUE (profile_name);


--
-- Name: iax_settings_iax_id_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY iax_settings
    ADD CONSTRAINT iax_settings_iax_id_key UNIQUE (iax_id, param_name);


--
-- Name: iax_settings_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY iax_settings
    ADD CONSTRAINT iax_settings_pkey PRIMARY KEY (id);


--
-- Name: ivr_conf_name_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY ivr_conf
    ADD CONSTRAINT ivr_conf_name_key UNIQUE (name);


--
-- Name: ivr_conf_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY ivr_conf
    ADD CONSTRAINT ivr_conf_pkey PRIMARY KEY (id);


--
-- Name: ivr_entries_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY ivr_entries
    ADD CONSTRAINT ivr_entries_pkey PRIMARY KEY (id);


--
-- Name: lcr_conf_param_name_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY lcr_conf
    ADD CONSTRAINT lcr_conf_param_name_key UNIQUE (param_name);


--
-- Name: lcr_conf_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY lcr_conf
    ADD CONSTRAINT lcr_conf_pkey PRIMARY KEY (id);


--
-- Name: lcr_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY lcr
    ADD CONSTRAINT lcr_pkey PRIMARY KEY (id);


--
-- Name: lcr_profiles_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY lcr_profiles
    ADD CONSTRAINT lcr_profiles_pkey PRIMARY KEY (id);


--
-- Name: lcr_profiles_profile_name_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY lcr_profiles
    ADD CONSTRAINT lcr_profiles_profile_name_key UNIQUE (profile_name);


--
-- Name: lcr_settings_lcr_id_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY lcr_settings
    ADD CONSTRAINT lcr_settings_lcr_id_key UNIQUE (lcr_id, param_name);


--
-- Name: lcr_settings_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY lcr_settings
    ADD CONSTRAINT lcr_settings_pkey PRIMARY KEY (id);


--
-- Name: limit_conf_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY limit_conf
    ADD CONSTRAINT limit_conf_pkey PRIMARY KEY (id);


--
-- Name: local_stream_conf_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY local_stream_conf
    ADD CONSTRAINT local_stream_conf_pkey PRIMARY KEY (id);


--
-- Name: modless_conf_conf_name_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY modless_conf
    ADD CONSTRAINT modless_conf_conf_name_key UNIQUE (conf_name);


--
-- Name: modless_conf_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY modless_conf
    ADD CONSTRAINT modless_conf_pkey PRIMARY KEY (id);


--
-- Name: npa_nxx_company_ocn_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY npa_nxx_company_ocn
    ADD CONSTRAINT npa_nxx_company_ocn_pkey PRIMARY KEY (npa, nxx);


--
-- Name: post_load_modules_conf_module_name_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY post_load_modules_conf
    ADD CONSTRAINT post_load_modules_conf_module_name_key UNIQUE (module_name);


--
-- Name: post_load_modules_conf_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY post_load_modules_conf
    ADD CONSTRAINT post_load_modules_conf_pkey PRIMARY KEY (id);


--
-- Name: rss_conf_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY rss_conf
    ADD CONSTRAINT rss_conf_pkey PRIMARY KEY (id);


--
-- Name: sofia_aliases_alias_name_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY sofia_aliases
    ADD CONSTRAINT sofia_aliases_alias_name_key UNIQUE (alias_name);


--
-- Name: sofia_aliases_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY sofia_aliases
    ADD CONSTRAINT sofia_aliases_pkey PRIMARY KEY (id);


--
-- Name: sofia_conf_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY sofia_conf
    ADD CONSTRAINT sofia_conf_pkey PRIMARY KEY (id);


--
-- Name: sofia_conf_profile_name_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY sofia_conf
    ADD CONSTRAINT sofia_conf_profile_name_key UNIQUE (profile_name);


--
-- Name: sofia_domains_domain_name_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY sofia_domains
    ADD CONSTRAINT sofia_domains_domain_name_key UNIQUE (domain_name);


--
-- Name: sofia_domains_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY sofia_domains
    ADD CONSTRAINT sofia_domains_pkey PRIMARY KEY (id);


--
-- Name: sofia_gateways_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY sofia_gateways
    ADD CONSTRAINT sofia_gateways_pkey PRIMARY KEY (id);


--
-- Name: sofia_settings_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY sofia_settings
    ADD CONSTRAINT sofia_settings_pkey PRIMARY KEY (id);


--
-- Name: sofia_settings_sofia_id_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY sofia_settings
    ADD CONSTRAINT sofia_settings_sofia_id_key UNIQUE (sofia_id, param_name);


--
-- Name: voicemail_conf_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY voicemail_conf
    ADD CONSTRAINT voicemail_conf_pkey PRIMARY KEY (id);


--
-- Name: voicemail_conf_vm_profile_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY voicemail_conf
    ADD CONSTRAINT voicemail_conf_vm_profile_key UNIQUE (vm_profile);


--
-- Name: voicemail_email_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY voicemail_email
    ADD CONSTRAINT voicemail_email_pkey PRIMARY KEY (id);


--
-- Name: voicemail_email_voicemail_id_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY voicemail_email
    ADD CONSTRAINT voicemail_email_voicemail_id_key UNIQUE (voicemail_id, param_name);


--
-- Name: voicemail_settings_pkey; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY voicemail_settings
    ADD CONSTRAINT voicemail_settings_pkey PRIMARY KEY (id);


--
-- Name: voicemail_settings_voicemail_id_key; Type: CONSTRAINT; Schema: public; Owner: freeswitch; Tablespace: 
--

ALTER TABLE ONLY voicemail_settings
    ADD CONSTRAINT voicemail_settings_voicemail_id_key UNIQUE (voicemail_id, param_name);


--
-- Name: digits_rate; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX digits_rate ON lcr USING btree (digits, rate);


--
-- Name: fki_; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX fki_ ON acl_nodes USING btree (list_id);


--
-- Name: fki_lcr_profile; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX fki_lcr_profile ON lcr USING btree (lcr_profile);


--
-- Name: gateway; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE UNIQUE INDEX gateway ON carrier_gateway USING btree (prefix, suffix);


--
-- Name: profile_digits_15; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX profile_digits_15 ON lcr USING btree (digits, lcr_profile);


--
-- Name: sa_hostname; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sa_hostname ON sip_authentication USING btree (hostname);


--
-- Name: sa_nonce; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sa_nonce ON sip_authentication USING btree (nonce);


--
-- Name: sd_hostname; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sd_hostname ON sip_dialogs USING btree (hostname);


--
-- Name: sd_uuid; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sd_uuid ON sip_dialogs USING btree (uuid);


--
-- Name: sp_hostname; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sp_hostname ON sip_presence USING btree (hostname);


--
-- Name: sr_call_id; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sr_call_id ON sip_registrations USING btree (call_id);


--
-- Name: sr_contact; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sr_contact ON sip_registrations USING btree (contact);


--
-- Name: sr_expires; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sr_expires ON sip_registrations USING btree (expires);


--
-- Name: sr_hostname; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sr_hostname ON sip_registrations USING btree (hostname);


--
-- Name: sr_network_ip; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sr_network_ip ON sip_registrations USING btree (network_ip);


--
-- Name: sr_network_port; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sr_network_port ON sip_registrations USING btree (network_port);


--
-- Name: sr_presence_hosts; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sr_presence_hosts ON sip_registrations USING btree (presence_hosts);


--
-- Name: sr_profile_name; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sr_profile_name ON sip_registrations USING btree (profile_name);


--
-- Name: sr_sip_host; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sr_sip_host ON sip_registrations USING btree (sip_host);


--
-- Name: sr_sip_realm; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sr_sip_realm ON sip_registrations USING btree (sip_realm);


--
-- Name: sr_sip_user; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sr_sip_user ON sip_registrations USING btree (sip_user);


--
-- Name: sr_sip_username; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sr_sip_username ON sip_registrations USING btree (sip_username);


--
-- Name: sr_status; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX sr_status ON sip_registrations USING btree (status);


--
-- Name: ss_call_id; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ss_call_id ON sip_subscriptions USING btree (call_id);


--
-- Name: ss_event; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ss_event ON sip_subscriptions USING btree (event);


--
-- Name: ss_hostname; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ss_hostname ON sip_subscriptions USING btree (hostname);


--
-- Name: ss_presence_hosts; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ss_presence_hosts ON sip_subscriptions USING btree (presence_hosts);


--
-- Name: ss_proto; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ss_proto ON sip_subscriptions USING btree (proto);


--
-- Name: ss_sip_host; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ss_sip_host ON sip_subscriptions USING btree (sip_host);


--
-- Name: ss_sip_user; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ss_sip_user ON sip_subscriptions USING btree (sip_user);


--
-- Name: ss_sub_to_host; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ss_sub_to_host ON sip_subscriptions USING btree (sub_to_host);


--
-- Name: ss_sub_to_user; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ss_sub_to_user ON sip_subscriptions USING btree (sub_to_user);


--
-- Name: ssa_aor; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ssa_aor ON sip_shared_appearance_subscriptions USING btree (aor);


--
-- Name: ssa_hostname; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ssa_hostname ON sip_shared_appearance_subscriptions USING btree (hostname);


--
-- Name: ssa_profile_name; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ssa_profile_name ON sip_shared_appearance_subscriptions USING btree (profile_name);


--
-- Name: ssa_subscriber; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ssa_subscriber ON sip_shared_appearance_subscriptions USING btree (subscriber);


--
-- Name: ssd_call_id; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ssd_call_id ON sip_shared_appearance_dialogs USING btree (call_id);


--
-- Name: ssd_contact_str; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ssd_contact_str ON sip_shared_appearance_dialogs USING btree (contact_str);


--
-- Name: ssd_expires; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ssd_expires ON sip_shared_appearance_dialogs USING btree (expires);


--
-- Name: ssd_hostname; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ssd_hostname ON sip_shared_appearance_dialogs USING btree (hostname);


--
-- Name: ssd_profile_name; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX ssd_profile_name ON sip_shared_appearance_dialogs USING btree (profile_name);


--
-- Name: unique_route; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX unique_route ON lcr USING btree (digits, carrier_id);


--
-- Name: voicemail_msgs_idx1; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX voicemail_msgs_idx1 ON voicemail_msgs USING btree (created_epoch);


--
-- Name: voicemail_msgs_idx2; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX voicemail_msgs_idx2 ON voicemail_msgs USING btree (username);


--
-- Name: voicemail_msgs_idx3; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX voicemail_msgs_idx3 ON voicemail_msgs USING btree (domain);


--
-- Name: voicemail_msgs_idx4; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX voicemail_msgs_idx4 ON voicemail_msgs USING btree (uuid);


--
-- Name: voicemail_msgs_idx5; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX voicemail_msgs_idx5 ON voicemail_msgs USING btree (in_folder);


--
-- Name: voicemail_msgs_idx6; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX voicemail_msgs_idx6 ON voicemail_msgs USING btree (read_flags);


--
-- Name: voicemail_prefs_idx1; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX voicemail_prefs_idx1 ON voicemail_prefs USING btree (username);


--
-- Name: voicemail_prefs_idx2; Type: INDEX; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE INDEX voicemail_prefs_idx2 ON voicemail_prefs USING btree (domain);


--
-- Name: acl_nodes_list_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY acl_nodes
    ADD CONSTRAINT acl_nodes_list_id_fkey FOREIGN KEY (list_id) REFERENCES acl_lists(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: carrier_gateway_carrier_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY carrier_gateway
    ADD CONSTRAINT carrier_gateway_carrier_id_fkey FOREIGN KEY (carrier_id) REFERENCES carriers(id);


--
-- Name: dialplan_actions_condition_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY dialplan_actions
    ADD CONSTRAINT dialplan_actions_condition_id_fkey FOREIGN KEY (condition_id) REFERENCES dialplan_condition(condition_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: dialplan_condition_extension_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY dialplan_condition
    ADD CONSTRAINT dialplan_condition_extension_id_fkey FOREIGN KEY (extension_id) REFERENCES dialplan_extension(extension_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: dialplan_context_dialplan_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY dialplan_context
    ADD CONSTRAINT dialplan_context_dialplan_id_fkey FOREIGN KEY (dialplan_id) REFERENCES dialplan(dialplan_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: dialplan_extension_context_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY dialplan_extension
    ADD CONSTRAINT dialplan_extension_context_id_fkey FOREIGN KEY (context_id) REFERENCES dialplan_context(context_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: dingaling_profile_params_dingaling_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY dingaling_profile_params
    ADD CONSTRAINT dingaling_profile_params_dingaling_id_fkey FOREIGN KEY (dingaling_id) REFERENCES dingaling_profiles(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: directory_gateway_params_d_gw_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY directory_gateway_params
    ADD CONSTRAINT directory_gateway_params_d_gw_id_fkey FOREIGN KEY (d_gw_id) REFERENCES directory_gateways(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: directory_gateways_directory_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY directory_gateways
    ADD CONSTRAINT directory_gateways_directory_id_fkey FOREIGN KEY (directory_id) REFERENCES directory(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: directory_global_params_directory_id_fkey1; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY directory_global_params
    ADD CONSTRAINT directory_global_params_directory_id_fkey1 FOREIGN KEY (directory_id) REFERENCES directory_domains(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: directory_global_vars_directory_id_fkey1; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY directory_global_vars
    ADD CONSTRAINT directory_global_vars_directory_id_fkey1 FOREIGN KEY (directory_id) REFERENCES directory_domains(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: directory_params_directory_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY directory_params
    ADD CONSTRAINT directory_params_directory_id_fkey FOREIGN KEY (directory_id) REFERENCES directory(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: directory_vars_directory_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY directory_vars
    ADD CONSTRAINT directory_vars_directory_id_fkey FOREIGN KEY (directory_id) REFERENCES directory(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: iax_settings_iax_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY iax_settings
    ADD CONSTRAINT iax_settings_iax_id_fkey FOREIGN KEY (iax_id) REFERENCES iax_conf(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: ivr_entries_ivr_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY ivr_entries
    ADD CONSTRAINT ivr_entries_ivr_id_fkey FOREIGN KEY (ivr_id) REFERENCES ivr_conf(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: lcr_carrier_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY lcr
    ADD CONSTRAINT lcr_carrier_id_fkey FOREIGN KEY (carrier_id) REFERENCES carriers(id);


--
-- Name: lcr_profile; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY lcr
    ADD CONSTRAINT lcr_profile FOREIGN KEY (lcr_profile) REFERENCES lcr_profiles(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: lcr_settings_lcr_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY lcr_settings
    ADD CONSTRAINT lcr_settings_lcr_id_fkey FOREIGN KEY (lcr_id) REFERENCES lcr_profiles(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: sofia_aliases_sofia_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY sofia_aliases
    ADD CONSTRAINT sofia_aliases_sofia_id_fkey FOREIGN KEY (sofia_id) REFERENCES sofia_conf(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: sofia_domains_sofia_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY sofia_domains
    ADD CONSTRAINT sofia_domains_sofia_id_fkey FOREIGN KEY (sofia_id) REFERENCES sofia_conf(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: sofia_gateways_sofia_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY sofia_gateways
    ADD CONSTRAINT sofia_gateways_sofia_id_fkey FOREIGN KEY (sofia_id) REFERENCES sofia_conf(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: sofia_settings_sofia_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY sofia_settings
    ADD CONSTRAINT sofia_settings_sofia_id_fkey FOREIGN KEY (sofia_id) REFERENCES sofia_conf(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: voicemail_email_voicemail_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY voicemail_email
    ADD CONSTRAINT voicemail_email_voicemail_id_fkey FOREIGN KEY (voicemail_id) REFERENCES voicemail_conf(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: voicemail_settings_voicemail_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: freeswitch
--

ALTER TABLE ONLY voicemail_settings
    ADD CONSTRAINT voicemail_settings_voicemail_id_fkey FOREIGN KEY (voicemail_id) REFERENCES voicemail_conf(id) ON UPDATE CASCADE ON DELETE CASCADE;


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

