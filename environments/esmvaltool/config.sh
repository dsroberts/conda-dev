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
export ENVIRONMENT=esmvaltool
version=2.10
export FULLENV="${ENVIRONMENT}-${version}"

declare -a rpms_to_remove=( "openssh-clients" "openssh-server" "openssh" )
declare -a replace_from_apps=( "ucx/1.14.0" )
declare -a outside_commands_to_include=( "pbs_tmrsh" "ssh" )
declare -a outside_files_to_copy=()