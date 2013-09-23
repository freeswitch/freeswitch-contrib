--
-- Name: phone_features; Type: TABLE; Schema: public; Owner: freeswitch; Tablespace: 
--

CREATE TABLE phone_features (
    id integer NOT NULL,
    dnd boolean DEFAULT false NOT NULL,
    cfwd_immediate_number character varying(255),
    cfwd_busy_number character varying(255),
    cfwd_no_answer_number character varying(255),
    cfwd_no_answer_rings integer DEFAULT 0 NOT NULL,
    username character varying(255) NOT NULL,
    domain character varying(255) NOT NULL,
    cfwd_immediate_active boolean DEFAULT false NOT NULL,
    cfwd_busy_active boolean DEFAULT false NOT NULL,
    cfwd_no_answer_active boolean DEFAULT false NOT NULL
);


ALTER TABLE public.phone_features OWNER TO freeswitch;

--
-- Name: phone_features_id_seq; Type: SEQUENCE; Schema: public; Owner: freeswitch
--

CREATE SEQUENCE phone_features_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.phone_features_id_seq OWNER TO freeswitch;

--
-- Name: phone_features_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: freeswitch
--

ALTER SEQUENCE phone_features_id_seq OWNED BY phone_features.id;


--
-- Name: phone_features_id_seq; Type: SEQUENCE SET; Schema: public; Owner: freeswitch
--

SELECT pg_catalog.setval('phone_features_id_seq', 6, true);


