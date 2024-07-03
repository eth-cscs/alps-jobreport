#ifndef JOBREPORT_MACROS_HPP
#define JOBREPORT_MACROS_HPP

#define ROOT_METADATA_FILE ".cscs_jobreport_root"
#define CONTAINER_HOOK_DEFAULT_IN_HOME ".config/enroot/hooks.d/cscs_jobreport_dcgm_hook.sh"

#define ENROOT_HOOK "#!/usr/bin/env bash\n\
\n\
set -euo pipefail\n\
shopt -s lastpipe nullglob\n\
\n\
export PATH=\"${PATH}:/usr/sbin:/sbin\"\n\
\n\
source \"${ENROOT_LIBRARY_PATH}/common.sh\"\n\
\n\
common::checkcmd grep sed ldd ldconfig\n\
\n\
if [ \"${OCI_ANNOTATION_com__hooks__dcgm__enabled:-}\" != \"true\" ]; then\n\
    exit 0\n\
fi\n\
\n\
# Mounting the specified DCGM libraries and directories explicitly\n\
cat << EOF | enroot-mount --root \"${ENROOT_ROOTFS}\" -\n\
/usr/local/dcgm /usr/local/dcgm none x-create=dir,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libnvperf_dcgm_host.so /usr/lib64/libnvperf_dcgm_host.so none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmodulesysmon.so.3.3.6 /usr/lib64/libdcgmmodulesysmon.so.3.3.6 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmodulesysmon.so.3 /usr/lib64/libdcgmmodulesysmon.so.3 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmodulesysmon.so /usr/lib64/libdcgmmodulesysmon.so none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmoduleprofiling.so.3.3.6 /usr/lib64/libdcgmmoduleprofiling.so.3.3.6 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmoduleprofiling.so.3 /usr/lib64/libdcgmmoduleprofiling.so.3 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmoduleprofiling.so /usr/lib64/libdcgmmoduleprofiling.so none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmodulepolicy.so.3.3.6 /usr/lib64/libdcgmmodulepolicy.so.3.3.6 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmodulepolicy.so.3 /usr/lib64/libdcgmmodulepolicy.so.3 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmodulepolicy.so /usr/lib64/libdcgmmodulepolicy.so none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmodulenvswitch.so.3.3.6 /usr/lib64/libdcgmmodulenvswitch.so.3.3.6 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmodulenvswitch.so.3 /usr/lib64/libdcgmmodulenvswitch.so.3 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmodulenvswitch.so /usr/lib64/libdcgmmodulenvswitch.so none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmoduleintrospect.so.3.3.6 /usr/lib64/libdcgmmoduleintrospect.so.3.3.6 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmoduleintrospect.so.3 /usr/lib64/libdcgmmoduleintrospect.so.3 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmoduleintrospect.so /usr/lib64/libdcgmmoduleintrospect.so none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmodulehealth.so.3.3.6 /usr/lib64/libdcgmmodulehealth.so.3.3.6 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmodulehealth.so.3 /usr/lib64/libdcgmmodulehealth.so.3 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmodulehealth.so /usr/lib64/libdcgmmodulehealth.so none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmodulediag.so.3.3.6 /usr/lib64/libdcgmmodulediag.so.3.3.6 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmodulediag.so.3 /usr/lib64/libdcgmmodulediag.so.3 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmodulediag.so /usr/lib64/libdcgmmodulediag.so none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmoduleconfig.so.3.3.6 /usr/lib64/libdcgmmoduleconfig.so.3.3.6 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmoduleconfig.so.3 /usr/lib64/libdcgmmoduleconfig.so.3 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgmmoduleconfig.so /usr/lib64/libdcgmmoduleconfig.so none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgm_stub.a /usr/lib64/libdcgm_stub.a none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgm_cublas_proxy12.so /usr/lib64/libdcgm_cublas_proxy12.so none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgm_cublas_proxy11.so /usr/lib64/libdcgm_cublas_proxy11.so none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgm.so.3.3.6 /usr/lib64/libdcgm.so.3.3.6 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgm.so.3 /usr/lib64/libdcgm.so.3 none x-create=file,bind,ro,nosuid,nodev,private\n\
/usr/lib64/libdcgm.so /usr/lib64/libdcgm.so none x-create=file,bind,ro,nosuid,nodev,private\n\
EOF\n\
\n\
# Refresh the dynamic linker cache to include newly mounted libs\n\
cat << EOF > \"${ENROOT_ROOTFS}/etc/ld.so.conf.d/enroot-dcgm-hook.conf\"\n\
/lib64\n\
/usr/lib64\n\
EOF\n\
\n\
if ! ${ldconfig:-ldconfig} -r \"${ENROOT_ROOTFS}\" >> \"${ENROOT_ROOTFS}/dcgm-hook.log\" 2>&1; then\n\
    common::err \"Failed to refresh the dynamic linker cache\"\n\
fi\n"

#endif // JOBREPORT_MACROS_HPP