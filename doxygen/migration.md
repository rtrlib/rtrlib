Migration/Implementation Guide for ASPA Support in RTRlib
==============

**Authors:** <tassilo.tanneberger@tu-dresden.de> <matthias.braeuer@haw-hamburg.de>

For our latest release, we unfortunately had to make some breaking changes to our API to accustom the new ASPA implementation. This document will guide you through the breaking- and other relevant changes.

## Initialization

**Old**
```c
if (rtr_mgr_init(&conf, groups, 1, 30, 600, 600, NULL, NULL, &connection_status_callback, NULL) < 0) {
  return EXIT_FAILURE;
}
```

**New**
```c
if (rtr_mgr_init(&conf, groups, 1, &connection_status_callback, NULL) < 0) {
  return EXIT_FAILURE;
}	

if (rtr_mgr_add_roa_support(conf, NULL) == RTR_ERROR) {
  fprintf(stderr, "Failed initializing ROA support\n");
  return EXIT_FAILURE;
}

if (rtr_mgr_add_aspa_support(conf, NULL) == RTR_ERROR) {
  fprintf(stderr, "Failed initializing ASPA support\n");
  return EXIT_FAILURE;
}

if (rtr_mgr_add_spki_support(conf, NULL) == RTR_ERROR) {
  fprintf(stderr, "Failed initializing BGPSEC support\n");
  return EXIT_FAILURE;
}

rtr_mgr_setup_sockets(conf, groups, 1, 50, 600, 600);
```

The `refresh_interval`, `expire_interval` and `retry_interval` for the sockets have been moved to the `rtr_mgr_setup_sockets` function. We have also restructured the initialization API so that programmers have more control over what kind of RPKI objects should be handled by RTRlib. Users can enable support for specific RPKI objects by calling the `rtr_mgr_add_{roa, aspa, spki}_support(conf, update_fp)` method. If the user does not enable support, these RPKI objects are discarded and methods that access this data return an error.

### Restructured Validation API

Since ROA and SPKI support can now be switched on and off individually, we decided to make our validation API more uniform. 
**Old**

```c
enum pfxv_state pfx_validation_result;
int pfx_ret = rtr_mgr_validate(conf, asn, prefix, mask_length, pfx_validation_result);

if (pfx_ret == PFX_SUCCESS) {
  // now check pfx_validation_result
}

struct rtr_bgpsec data;
int bgpsec_ret = rtr_mgr_bgpsec_validate(&data, conf);

if (bgpsec_ret == RTR_BGPSEC_VALID) {
  // 
}
```

**New**
```c

enum pfxv_state pfx_result;
enum pfx_ret = rtr_mgr_pfx_validate(conf, asn, prefix, mask_length, pfx_validation_result)
if (pfx_ret == ROA_UNINITIALIZED) {

}

enum bgpsec_validation_result bgpsec_result;
enum bgpsec_ret = rtr_mgr_bgpsec_validate()
if (bgpsec_ret == BGPSEC_UNINITIALIZED) {

}
```

If a support for particular RPKI object was not added the methods return `ROA_NOT_INITIALIZED` or `BGPSEC_NOT_INITIALIZED` respectively. If the methods return `SUCCESS` the result of the validation can 

## Using RTRlib to validate AS_PATHs using ASPA 

The validation of the AS_PATH in incoming BGP announcements is carried out with the function ```rtr_mgr_aspa_validate(aspa_table, as_path, as_path_size, direction)```. This function takes a pointer to the `aspa_table`, which you can obtain from the `rtr_mgr_config` struct (`conf->aspa_table`). The second argument is the AS_PATH, a pointer to a `uint32_t` array and the third argument is the length of the as path (not the byte length). The last argument is the direction of the BGP announcement. If the BGP announcement was received from one of your customers, the input should be `ASPA_UPSTREAM`, if you receive the BGP announcement from one of your providers, the input should be `ASPA_DOWNSTREAM`.

The return value of the function is either `ASPA_AS_PATH_VALID`, which means that there is proofably no route leak in the AS_PATH, or `ASPA_AS_PATH_INVALID` means that there is a route leak in the AS_PATH. If a route leak is detected, the BGP announcement should be dropped. The [draft](https://datatracker.ietf.org/doc/draft-ietf-sidrops-aspa-verification/) recommends that `ASPA_AS_PATH_UNKNOWN` should be equally treated to `ASPA_AS_PATH_VALID`. `ASPA_AS_PATH_UNKNOWN` means that the algorithm did not have enough data to come to reach definitive conclusion.


