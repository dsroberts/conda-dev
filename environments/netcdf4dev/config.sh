### config.sh MUST provide the following:
### $ENVIRONMENT
### $FULLENV
###
### Arrays (can be empty)
### rpms_to_remove
### replace_from_apps
### outside_commands_to_include
### outside_files_to_copy

### Version settings
export FULLENV=netcdf4dev

declare -a rpms_to_remove=()
declare -a replace_from_apps=()
declare -a outside_commands_to_include=()
declare -a outside_files_to_copy=()