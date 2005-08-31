-- notes:
--  Dont use apostrophes in comments
--  Dont use variable names that match column names when setting variable (see my_* usages)
--  primative logging by raising notices. level debug is useless w/o modifications to postgres.conf
--  some functions preserve primary keys from PDS, others do not.  depends on how easy it is to adjust
--    and if destination table has to merge values from multiple tables in old schema.  if primary keys
--    are preserved, sequences must be updated manually
--
-- todos:
-- bail on any error
-- workout logging that can keep a audit trail of migration incase there was a bad translation
--   discovered post migration AND after users have added new data and running another migration is
--   not possible and custom repair script can be written

delete from line;
delete from phone_group;
delete from phone;

-- todo, translate superuser as well
delete from user_group;
delete from users where user_id != 1;

delete from group_storage;
delete from setting_value;
delete from value_storage;

-- U S E R   G R O U P S
create or replace function migrate_user_groups() returns integer as '
declare
  usrgrp record;
  next_id int;
begin

  for usrgrp in select * from dblink(''select id, name from user_groups'') as (id int, name text) loop

    raise notice ''importing user group %...'', usrgrp.name;
 
    insert into value_storage (value_storage_id) values (usrgrp.id);

    -- todo weights ought to be the lower the depth, the higher to weight
    -- how do you find the depth though?

    insert into group_storage (group_id, name, weight, resource)
        values (usrgrp.id, usrgrp.name, nextval(''group_weight_seq''), ''user'');

    -- todo user permissions
    
  end loop; 

  -- update value_storage_seq  
  next_id := max(value_storage_id) + 1 from value_storage;
  perform setval(''storage_seq'', next_id);

  return 1;
end;
' language plpgsql;

-- U S E R S
create or replace function migrate_users() returns integer as '
declare
  usr record;
  next_id int;
begin

  -- avoid SDS and superadmin users
  for usr in select * from dblink(''select id, first_name, password, last_name, display_id, extension, ug_id from users where ug_id is not null'') as (id int, first_name text, password text, last_name text, display_id text, extension text, ug_id int) loop

   raise notice ''importing user %...'', usr.display_id;

   insert into users (user_id, first_name, pintoken, last_name, user_name)
      values (usr.id, usr.first_name, usr.password, usr.last_name, usr.display_id);

   -- user group
   insert into user_group (user_id, group_id) values (usr.id, usr.ug_id);

   -- todo report user permissions that cannot be handled

   -- todo update user_seq

   -- todo add all groups that was parent to current group

  end loop; 

  next_id := max(user_id) + 1 from users;
  perform setval(''user_seq'', next_id);

  return 1;
end;
' language plpgsql;


-- U S E R  G R O U P  H I E R A R C H Y
create or replace function migrate_user_group_tree() returns integer as '
declare
  not_supported_notice record;
begin

  -- level 1
  perform insert_users_into_groups(''
  select 
    u.id, g1.id from users u, user_groups g, user_groups g1
  where 
    u.ug_id = g.id and g.parent_ug_id = g1.id'');

  -- level 2
  perform insert_users_into_groups(''
  select 
    u.id, g2.id from users u, user_groups g, user_groups g1, user_groups g2
  where 
    u.ug_id = g.id and g.parent_ug_id = g1.id and g1.parent_ug_id = g2.id'');

  -- level 3
  perform insert_users_into_groups(''
  select 
    u.id, g3.id from users u, user_groups g, user_groups g1, user_groups g2, user_groups g3
  where 
    u.ug_id = g.id and g.parent_ug_id = g1.id and g1.parent_ug_id = g2.id and 
    g2.parent_ug_id = g3.id'');

  -- level 4 and above not supported
  select into not_supported_notice * from dblink(''
  select 
    u.id, g4.id from users u, user_groups g, user_groups g1, user_groups g2, user_groups g3, user_groups g4
  where 
    u.ug_id = g.id and g.parent_ug_id = g1.id and g1.parent_ug_id = g2.id and 
    g2.parent_ug_id = g3.id and g3.parent_ug_id = g4.id'') 
  as (user_id int, group_id int);
  if found then
    raise exception ''cannot support group levels 4 parents or deeper'';
  end if;

  return 1;
end;
' language plpgsql;

-- help function for user tree function
create or replace function insert_users_into_groups(varchar) returns integer as '
declare
  user_select alias for $1;
  grp record;
begin
  for grp in select * from dblink(user_select) as (user_id int, group_id int) loop
    insert into user_group (user_id, group_id) values (grp.user_id, grp.group_id);
  end loop;

  return 1;
end;
' language plpgsql;

-- P H O N E  G R O U P S
create or replace function migrate_phone_groups() returns integer as '
declare
  phonegrp record;
  next_id int;
begin

  -- cannot insert phone group id directly into new database, ids would conflict with
  -- user groups as the share same tables
  create temporary table phone_group_migration (pds_group_id int not null, group_id int not null);

  for phonegrp in select * from dblink(''select id, name from phone_groups'') as (id int, name text) loop

    next_id := nextval(''storage_seq'');

    insert into phone_group_migration (pds_group_id, group_id) values (phonegrp.id, next_id);

    insert into value_storage (value_storage_id) values (next_id);

    insert into group_storage (group_id, name, weight, resource)
        values (next_id, phonegrp.name, nextval(''group_weight_seq''), ''phone'');

  end loop; 

  return 1;
end;
' language plpgsql;

-- P H O N E   G R O U P  H I E R A R C H Y
create or replace function migrate_phone_group_tree() returns integer as '
declare
  not_supported_notice record;
begin

  -- level 1
  perform insert_phones_into_groups(''
  select 
    p.id, g1.id from logical_phones p, phone_groups g, phone_groups g1
  where 
    p.pg_id = g.id and g.parent_pg_id = g1.id'');

  -- level 2
  perform insert_phones_into_groups(''
  select 
    p.id, g2.id from logical_phones p, phone_groups g, phone_groups g1, phone_groups g2
  where 
    p.pg_id = g.id and g.parent_pg_id = g1.id and g1.parent_pg_id = g2.id'');

  -- level 3
  perform insert_phones_into_groups(''
  select 
    p.id, g3.id from logical_phones p, phone_groups g, phone_groups g1, phone_groups g2, phone_groups g3
  where 
    p.pg_id = g.id and g.parent_pg_id = g1.id and g1.parent_pg_id = g2.id and
    g2.parent_pg_id = g3.id'');

  -- level 4 and above not supported
  select into not_supported_notice * from dblink(''
  select 
    p.id, g4.id from logical_phones p, phone_groups g, phone_groups g1, phone_groups g2, phone_groups g3,
    phone_groups g4
  where 
    p.pg_id = g.id and g.parent_pg_id = g1.id and g1.parent_pg_id = g2.id and
    g2.parent_pg_id = g3.id and g3.parent_pg_id = g4.id'') 
  as (phone_id int, group_id int);
  if found then
    raise exception ''cannot support group levels 4 parents or deeper'';
  end if;

  return 1;
end;
' language plpgsql;

create or replace function insert_phones_into_groups(varchar) returns integer as '
declare
  phone_select alias for $1;
  grp record;
  phone_group_id int;
begin

  for grp in select * from dblink(phone_select) as (phone_id int, group_id int) loop
    select into phone_group_id group_id from phone_group_migration where pds_group_id = grp.group_id;
    insert into phone_group (phone_id, group_id) values (grp.phone_id, phone_group_id);
  end loop;

  return 1;
end;
' language plpgsql;


-- N O N   P O L Y C O M   P H O N E S
create or replace function migrate_non_polycom_phones() returns integer as '
declare
  phone record;
  my_group_id int;
  my_phone_type record;
  next_id int;
begin

  -- translation table for phone types
  create temporary table phone_type_migration ( pds_model text, bean_id text, model_id text);
  insert into phone_type_migration values (''7960'', ''ciscoIp'', ''7960'');
  insert into phone_type_migration values (''7940'', ''ciscoIp'', ''7940'');
  insert into phone_type_migration values (''xpressa_strongarm_vxworks'', ''genericPhone'', null);
  insert into phone_type_migration values (''ixpressa_x86_win32'', ''genericPhone'', null);

  for phone in select * from 
      dblink(''select p.id, p.serial_number, p.pg_id, p.usrs_id, pt.model 
      from logical_phones p, phone_types pt where p.pt_id = pt.id'') 
      as (id int, serial_number text, pg_id int, usrs_id int, pt_model text) loop

    select into my_phone_type * from phone_type_migration where pds_model = phone.pt_model; 

    raise notice ''importing phone %, %, %...'', phone.serial_number, my_phone_type.bean_id;

    insert into phone (phone_id, serial_number, bean_id, model_id) 
      values (phone.id, phone.serial_number, my_phone_type.bean_id, my_phone_type.model_id);

    select into my_group_id group_id from phone_group_migration where pds_group_id = phone.pg_id;

    insert into phone_group (phone_id, group_id) values (phone.id, my_group_id);

    if phone.usrs_id is not null 
    then
      insert into line (line_id, phone_id, position, user_id) 
        values (nextval(''line_seq''), phone.id, 0, phone.usrs_id);
    end if;

    -- todo, phone settings, only cisco? very difficult

  end loop; 

  -- update value_storage_seq  
  next_id := max(phone_id) + 1 from phone;
  perform setval(''phone_seq'', next_id);

  return 1;
end;
' language plpgsql;


-- P O L Y C O M   P H O N E S 
create or replace function migrate_polycom_phones() returns integer as '
declare
  phone record;
  line record;
  next_id int;
  default_group_id int;
begin

  select into default_group_id group_id from group_storage where name = ''default'' and resource = ''phone'';

  for phone in select * from dblink(''select p.phone_id, p.serial_number, p.name, p.factory_id, p.storage_id, p.folder_id from phone p'') as (id int, serial_number text, name text, factory_id text, storage_id int, folder_id int) loop
   
    next_id := nextval(''phone_seq'');
    insert into phone (phone_id, name, serial_number, value_storage_id, bean_id, model_id) 
      values (next_id, phone.name, phone.serial_number, phone.storage_id, ''polycom'',
              substring(phone.factory_id from ''\\\\d*$''));

    for line in select * from dblink(''select position, storage_id, user_id from 
        line where phone_id ='' || phone.id) as (position int, storage_id int, user_id int) loop
    
        insert into line (line_id, phone_id, position, user_id, value_storage_id) 
          values (nextval(''line_seq''), next_id, line.position, line.user_id, line.storage_id);
         
    end loop;
    
  end loop; 

  -- all polycom phones in default group implicitly 
  insert into phone_group 
    select p.phone_id, default_group_id from phone p where bean_id = ''polycom'';

  return 1;
end;
' language plpgsql;

-- S E T T I N G S
create or replace function migrate_settings() returns integer as '
declare
  next_id int;
begin

 -- straight migration 

 insert into value_storage select * from 
   dblink(''select storage_id from storage'') as (value_storage_id int);

  insert into setting_value (value_storage_id, path, value) select * from
    dblink(''select storage_id, path, value from setting'') 
    as (storage_id int, path text, value text);

  -- update value_storage_seq  
  next_id := max(value_storage_id) + 1 from value_storage;
  perform setval(''storage_seq'', next_id);
  
  return 1;
end;
' language plpgsql;



-- ********** END PL/pgSQL **************


-- ********** BEGIN SQL ****************

load 'dblink';

select dblink_connect('dbname=PDS');

-- todo migrate folder and folder values

select migrate_settings();
select migrate_user_groups();
select migrate_users();
select migrate_user_group_tree();
select migrate_phone_groups();
select migrate_non_polycom_phones();
select migrate_polycom_phones();
select migrate_phone_group_tree();
-- todo lines

