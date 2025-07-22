project(
    'gpio-monitor',
    'cpp',
    default_options: [
        'warning_level=3',
        'werror=true',
        'cpp_std=c++23',
        'buildtype=debugoptimized',
    ],
    license: 'Apache-2.0',
    version: '1.0',
    meson_version: '>=1.1.1',
)

cxx = meson.get_compiler('cpp')

# Dependencies
libevdev = dependency('libevdev')
libsystemd = dependency('libsystemd')
libgpiod = dependency('libgpiod')
phosphor_dbus_interfaces = dependency('phosphor-dbus-interfaces')
phosphor_logging = dependency('phosphor-logging')
sdbusplus = dependency('sdbusplus')
systemd = dependency('systemd')
udev = dependency('udev')
boost_dep = dependency('boost')

nlohmann_json_dep = dependency('nlohmann_json', include_type: 'system')

# CLI11 dependency fallback
if cxx.has_header('CLI/CLI.hpp')
    cli11_dep = declare_dependency()
else
    cli11_dep = dependency('CLI11')
endif

# Boost flags
boost_args = [
    '-DBOOST_ASIO_DISABLE_THREADS',
    '-DBOOST_ERROR_CODE_HEADER_ONLY',
    '-DBOOST_SYSTEM_NO_DEPRECATED',
]

# Install systemd service file
systemd_system_unit_dir = systemd.get_variable(
    'systemd_system_unit_dir',
    pkgconfig_define: ['prefix', get_option('prefix')],
)

fs = import('fs')

fs.copyfile(
    'service_files/gpio-monitor.service',
    install: true,
    install_dir: systemd_system_unit_dir,
)

# Install JSON config file
fs.copyfile(
    'config/gpio-monitor.json',
    install: true,
    install_dir: get_option('datadir') / 'gpio-monitor',
)

# Final executable build
executable(
    'gpio-monitor',
    'src/gpioMonMain.cpp',
    'src/gpioMon.cpp',
    dependencies: [
        cli11_dep,
        libgpiod,
        nlohmann_json_dep,
        phosphor_dbus_interfaces,
        phosphor_logging,
        sdbusplus,
        boost_dep,
    ],
    cpp_args: boost_args,
    install: true,
)