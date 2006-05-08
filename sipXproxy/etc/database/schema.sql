/*
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 */

/* 
 * Schema for Call Resolver data, including Call State Events (CSE) and
 * Call Detail Records (CDR).
 */


---------------------------------- Versioning ----------------------------------
/*
 * Follow the sipXconfig model for version history
 */

create table version_history(
  version int4 not null primary key,
  applied date not null
);

/**
 * CHANGE VERSION HERE ----------------------------> X <------------------
 *
 * For the initial sipX release with Call Resolver, the database version is 2.
 */
insert into version_history (version, applied) values (2, now());

create table patch(
  name varchar(32) not null primary key
);


---------------------------------- CSE Tables ----------------------------------

/*
 * The call_state_events holds events that describe the lifecycle of a call:
 * call request, call setup, call end or call failure.
 *
 * Values for event_type are
 *   'R' - Call Request, generated by the proxy server when an INVITE request is
 *         received that does not have a tag parameter on the TO field value.
 *   'S' - Call Setup, generated when an ACK is received.
 *   'E' - Call End, generated by any BYE request.
 *   'T' - Call Transfer, generated by an REFER request.
 *   'F' - Call Failure, generated by any 5xx or 6xx response to an INVITE
 *         request, or any 4xx response to an INVITE - except
 *                    
 *           401 Authentication Required
 *           407 Proxy Authentication Required
 *           408 Request timeout
 */
create table call_state_events (
   id              serial8 not null primary key,
   observer        text not null,       /* DNS name of the system that observed the event */
   event_seq       int8 not null,       /* Sequence number of this event at the observer */
   event_time      timestamp not null,  /* Observer (UTC) time when this event occurred */
   event_type      char(1) not null,
   cseq            int4 not null,       /* Value from CSEQ header */ 
   call_id         text not null,       /* Value from SIP Call-Id header */
   from_tag        text,                /* Value from tag attribute of the SIP From header */
   to_tag          text,                /* Value from tag attribute of the SIP To header */
   from_url        text not null,       /* Full From header field value */
   to_url          text not null,       /* Full To header field value */
   contact         text,    /* Contact header field value. For Call Request this is the
                               calling party, for Call Setup this is the called party. */
   refer_to        text,                /* Refer-To header field value */
   referred_by     text,                /* Referred-By header field value */
   failure_status  int2,    /* For Call Failure events, holds 4xx, 5xx, or 6xx status code */
   failure_reason  text,    /* For Call Failure events, holds error text */
   request_uri     text     /* URI from the request header */
);


/*
 * The observer_state_events table holds events relating to the event observer
 * state, for example, that the observer has restarted.  The forking proxy
 * and auth proxy are event observers.
 *
 * Status codes:
 *   101 - Observer Reset
 */
create table observer_state_events (
   id              serial8 not null primary key,
   observer        text not null,       /* DNS name of the system that observed the event */
   event_seq       int8 not null,       /* Sequence number of this event at the observer */
   event_time      timestamp not null,  /* Observer (UTC) time when this event occurred */
   status          int2 not null,       /* Status code */
   msg             text                 /* Explanatory message from observer */
);


---------------------------------- CDR Tables ----------------------------------

/*
 * The cdrs table records info about SIP calls.
 *
 * Start, connect, and end times are nullable to allow for partial CDRs where the
 * call cannot be completely analyzed.  For example, if we can't find a BYE or 
 * any other end event, then the end_time is indeterminate and will be set to NULL.
 *
 * Declaring call_id to be unique implicitly creates an index on that column, so
 * queries on call_id are fast.
 *
 * The termination code column holds an single char value indicating why the call
 * was terminated.  Codes are:
 * 
 *   R: call requested -- got a call_request event, but no other events.
 *   I: call in progress -- got both call_request and call_setup events.
 *   C: successful call completion -- call_request, call_setup, and call_end with
 *      no errors.
 *   F: call failed -- an error occurred.
 */
create table cdrs (
  id serial8 not null primary key,
  call_id text not null,            /* SIP call ID from the INVITE */
  from_tag text not null,           /* SIP from tag provided by caller in the INVITE */
  to_tag text not null,             /* SIP to tag provided by callee in the 200 OK */
  caller_aor text not null,         /* caller's SIP address of record (AOR) */
  callee_aor text not null,         /* callee's AOR */
  start_time timestamp,             /* Start time in GMT: initial INVITE received */
  connect_time timestamp,           /* Connect time in GMT: ACK received for 200 OK */
  end_time timestamp,               /* End time in GMT: BYE received, or other ending */
  termination char(1),              /* Why the call was terminated */
  failure_status int2,              /* SIP error code if the call failed, e.g., 4xx */
  failure_reason text,              /* Text describing the reason for a call failure */
  call_direction char(1)            /* Plugin feature, see below */
);

/*
 * Create a call_id index and a call_id uniqueness constraint.  Create the index
 * explicitly rather than relying on the implicit index generated by the uniqueness
 * constraint.  That makes it possible to turn uniqueness and indexing on/off
 * independently, which can be useful during bulk inserts.
 */
create index cdrs_call_id_index on cdrs (call_id);
alter table cdrs add constraint cdrs_call_id_unique unique (call_id);


---------------------------------- Views ----------------------------------

/*
 * Simplify the presentation of CDRs by showing commonly used CDR data in a
 * single view.
 * Do not include SIP dialog info in the view since it is not of interest for
 * billing.  Dialog info is only used to link the CDR back to raw CSE data,
 * or for CDR post-processing.
 */

create view view_cdrs as
  select id, caller_aor, callee_aor,
         start_time, connect_time, end_time,
         termination, failure_status, failure_reason
  from cdrs;


---------------------------------- Plugins ----------------------------------

/*
 * Call Direction
 */

/*
 * "Call direction" is an application-level plugin. 
 * The purist approach would be to put call direction in its own table, separate 
 * from the cdrs table, but that would be overkill just for a single char(1) column.
 * So this column is in the cdrs table, see above.
 *
 * Call direction is encoded in char(1) as follows: 
 *
 *   Incoming (I): for calls that come in from a PSTN gateway
 *   Outgoing (O): for calls that go out to a PSTN gateway
 *   Intranetwork (A): for calls that are pure SIP and don't go through a gateway
 */

/*
 * Define "view_cdrs_with_call_direction" to be a superset of "view_cdrs" defined 
 * above, with the addition of the call_direction column.
 */

create view view_cdrs_with_call_direction as
  select id, caller_aor, callee_aor,
         start_time, connect_time, end_time,
         termination, failure_status, failure_reason,
         call_direction
  from cdrs;

