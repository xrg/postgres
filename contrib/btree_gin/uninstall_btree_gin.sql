/* $PostgreSQL$ */

-- Adjust this setting to control where the objects get dropped.
SET search_path = public;

DROP OPERATOR FAMILY int2_ops USING gin CASCADE;
DROP OPERATOR FAMILY int4_ops USING gin CASCADE;
DROP OPERATOR FAMILY int8_ops USING gin CASCADE;
DROP OPERATOR FAMILY float4_ops USING gin CASCADE;
DROP OPERATOR FAMILY float8_ops USING gin CASCADE;
DROP OPERATOR FAMILY money_ops USING gin CASCADE;
DROP OPERATOR FAMILY oid_ops USING gin CASCADE;
DROP OPERATOR FAMILY timestamp_ops USING gin CASCADE;
DROP OPERATOR FAMILY timestamptz_ops USING gin CASCADE;
DROP OPERATOR FAMILY time_ops USING gin CASCADE;
DROP OPERATOR FAMILY timetz_ops USING gin CASCADE;
DROP OPERATOR FAMILY date_ops USING gin CASCADE;
DROP OPERATOR FAMILY interval_ops USING gin CASCADE;
DROP OPERATOR FAMILY macaddr_ops USING gin CASCADE;
DROP OPERATOR FAMILY inet_ops USING gin CASCADE;
DROP OPERATOR FAMILY cidr_ops USING gin CASCADE;
DROP OPERATOR FAMILY text_ops USING gin CASCADE;
DROP OPERATOR FAMILY varchar_ops USING gin CASCADE;
DROP OPERATOR FAMILY char_ops USING gin CASCADE;
DROP OPERATOR FAMILY bytea_ops USING gin CASCADE;
DROP OPERATOR FAMILY bit_ops USING gin CASCADE;
DROP OPERATOR FAMILY varbit_ops USING gin CASCADE;
DROP OPERATOR FAMILY numeric_ops USING gin CASCADE;

DROP FUNCTION gin_btree_consistent(internal, int2, anyelement, int4, internal, internal);

DROP FUNCTION gin_extract_value_int2(int2, internal);
DROP FUNCTION gin_compare_prefix_int2(int2, int2, int2, internal);
DROP FUNCTION gin_extract_query_int2(int2, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_int4(int4, internal);
DROP FUNCTION gin_compare_prefix_int4(int4, int4, int2, internal);
DROP FUNCTION gin_extract_query_int4(int4, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_int8(int8, internal);
DROP FUNCTION gin_compare_prefix_int8(int8, int8, int2, internal);
DROP FUNCTION gin_extract_query_int8(int8, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_float4(float4, internal);
DROP FUNCTION gin_compare_prefix_float4(float4, float4, int2, internal);
DROP FUNCTION gin_extract_query_float4(float4, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_float8(float8, internal);
DROP FUNCTION gin_compare_prefix_float8(float8, float8, int2, internal);
DROP FUNCTION gin_extract_query_float8(float8, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_money(money, internal);
DROP FUNCTION gin_compare_prefix_money(money, money, int2, internal);
DROP FUNCTION gin_extract_query_money(money, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_oid(oid, internal);
DROP FUNCTION gin_compare_prefix_oid(oid, oid, int2, internal);
DROP FUNCTION gin_extract_query_oid(oid, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_timestamp(timestamp, internal);
DROP FUNCTION gin_compare_prefix_timestamp(timestamp, timestamp, int2, internal);
DROP FUNCTION gin_extract_query_timestamp(timestamp, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_timestamptz(timestamptz, internal);
DROP FUNCTION gin_compare_prefix_timestamptz(timestamptz, timestamptz, int2, internal);
DROP FUNCTION gin_extract_query_timestamptz(timestamptz, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_time(time, internal);
DROP FUNCTION gin_compare_prefix_time(time, time, int2, internal);
DROP FUNCTION gin_extract_query_time(time, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_timetz(timetz, internal);
DROP FUNCTION gin_compare_prefix_timetz(timetz, timetz, int2, internal);
DROP FUNCTION gin_extract_query_timetz(timetz, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_date(date, internal);
DROP FUNCTION gin_compare_prefix_date(date, date, int2, internal);
DROP FUNCTION gin_extract_query_date(date, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_interval(interval, internal);
DROP FUNCTION gin_compare_prefix_interval(interval, interval, int2, internal);
DROP FUNCTION gin_extract_query_interval(interval, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_macaddr(macaddr, internal);
DROP FUNCTION gin_compare_prefix_macaddr(macaddr, macaddr, int2, internal);
DROP FUNCTION gin_extract_query_macaddr(macaddr, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_inet(inet, internal);
DROP FUNCTION gin_compare_prefix_inet(inet, inet, int2, internal);
DROP FUNCTION gin_extract_query_inet(inet, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_cidr(cidr, internal);
DROP FUNCTION gin_compare_prefix_cidr(cidr, cidr, int2, internal);
DROP FUNCTION gin_extract_query_cidr(cidr, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_text(text, internal);
DROP FUNCTION gin_compare_prefix_text(text, text, int2, internal);
DROP FUNCTION gin_extract_query_text(text, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_char("char", internal);
DROP FUNCTION gin_compare_prefix_char("char", "char", int2, internal);
DROP FUNCTION gin_extract_query_char("char", internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_bytea(bytea, internal);
DROP FUNCTION gin_compare_prefix_bytea(bytea, bytea, int2, internal);
DROP FUNCTION gin_extract_query_bytea(bytea, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_bit(bit, internal);
DROP FUNCTION gin_compare_prefix_bit(bit, bit, int2, internal);
DROP FUNCTION gin_extract_query_bit(bit, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_varbit(varbit, internal);
DROP FUNCTION gin_compare_prefix_varbit(varbit, varbit, int2, internal);
DROP FUNCTION gin_extract_query_varbit(varbit, internal, int2, internal, internal);
DROP FUNCTION gin_extract_value_numeric(numeric, internal);
DROP FUNCTION gin_compare_prefix_numeric(numeric, numeric, int2, internal);
DROP FUNCTION gin_extract_query_numeric(numeric, internal, int2, internal, internal);
DROP FUNCTION gin_numeric_cmp(numeric, numeric);
