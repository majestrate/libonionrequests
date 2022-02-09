local docker_base = 'registry.oxen.rocks/lokinet-ci-';

local submodule_commands = ['git fetch --tags', 'git submodule update --init --recursive --depth=1'];

local submodules = {
  name: 'submodules',
  image: 'drone/git',
  commands: submodule_commands,
};

local apt_get_quiet = 'apt-get -o=Dpkg::Use-Pty=0 -q ';

local debian_pipeline(name, image, arch='amd64', compiler_deps=['g++'], deps=['python3-dev', 'python3-pybind11', 'python3-pytest', 'libspdlog-dev', 'nlohmann-json3-dev', 'libssl-dev', 'libsodium-dev', 'liboxenmq-dev', 'liboxenc-dev'], cmake_extra='', build_type='Release', extra_cmds=[], allow_fail=false) = {
  kind: 'pipeline',
  type: 'docker',
  name: name,
  platform: { arch: arch },
  environment: { CLICOLOR_FORCE: '1' },  // Lets color through ninja (1.9+)
  steps: [
    submodules,
    {
      name: 'build',
      image: image,
      pull: 'always',
      [if allow_fail then 'failure']: 'ignore',
      commands: [
        'echo "Building on ${DRONE_STAGE_MACHINE}"',
        'echo "man-db man-db/auto-update boolean false" | debconf-set-selections',
        apt_get_quiet + 'update',
        apt_get_quiet + 'install -y eatmydata',
        'eatmydata ' + apt_get_quiet + ' install --no-install-recommends -y lsb-release',
        'cp contrib/deb.oxen.io.gpg /etc/apt/trusted.gpg.d',
        'echo deb http://deb.oxen.io $$(lsb_release -sc) main >/etc/apt/sources.list.d/oxen.list',
        'eatmydata ' + apt_get_quiet + ' update',
        'eatmydata ' + apt_get_quiet + 'dist-upgrade -y',
        'eatmydata ' + apt_get_quiet + 'install -y cmake git ninja-build pkg-config ccache ' + std.join(' ', deps) + ' ' + std.join(' ', compiler_deps),
        'mkdir build',
        'cd build',
        'cmake .. -DWITH_TESTS=ON -DWITH_PYBIND=ON -G Ninja -DCMAKE_CXX_FLAGS=-fdiagnostics-color=always -DCMAKE_BUILD_TYPE=' + build_type + ' -DCMAKE_CXX_COMPILER_LAUNCHER=ccache ' + cmake_extra,
        'ninja -v',
        'ninja -v check',
        'PYTHONPATH=pybind python3 -m pytest ../tests/',
      ] + extra_cmds,
    },
  ],
};

local clang(version) = debian_pipeline(
  'Debian sid/clang-' + version + ' (amd64)',
  docker_base + 'debian-sid-clang',
  compiler_deps=['clang-' + version],
  cmake_extra='-DCMAKE_C_COMPILER=clang-' + version + ' -DCMAKE_CXX_COMPILER=clang++-' + version + ' '
);

local full_llvm(version) = debian_pipeline(
  'Debian sid/llvm-' + version + ' (amd64)',
  docker_base + 'debian-sid-clang',
  compiler_deps=['clang-' + version, 'lld-' + version, 'libc++-' + version + '-dev', 'libc++abi-' + version + '-dev'],
  cmake_extra='-DCMAKE_C_COMPILER=clang-' + version +
              ' -DCMAKE_CXX_COMPILER=clang++-' + version +
              ' -DCMAKE_CXX_FLAGS=-stdlib=libc++ ' +
              std.join(' ', [
                '-DCMAKE_' + type + '_LINKER_FLAGS=-fuse-ld=lld-' + version
                for type in ['EXE', 'MODULE', 'SHARED']
              ])
);


[
  debian_pipeline('Debian sid (amd64)', docker_base + 'debian-sid'),
  debian_pipeline('Debian sid/Debug (amd64)', docker_base + 'debian-sid', build_type='Debug'),
  clang(13),
  //full_llvm(13),  // Fails at using ar when linking for some reason
  debian_pipeline('Debian bullseye (amd64)', docker_base + 'debian-bullseye'),
  debian_pipeline('Debian stable (i386)', docker_base + 'debian-stable/i386'),
  debian_pipeline('Debian sid (ARM64)', docker_base + 'debian-sid', arch='arm64'),
  debian_pipeline('Debian stable (armhf)', docker_base + 'debian-stable/arm32v7', arch='arm64'),
  //debian_pipeline('Ubuntu focal (amd64)', docker_base + 'ubuntu-focal'),
  {
    kind: 'pipeline',
    type: 'exec',
    name: 'macOS (w/macports)',
    platform: { os: 'darwin', arch: 'amd64' },
    environment: { CLICOLOR_FORCE: '1' },  // Lets color through ninja (1.9+)
    steps: [
      { name: 'submodules', commands: submodule_commands },
      {
        name: 'build',
        commands: [
          'mkdir build',
          'cd build',
          'export SDKROOT="$(xcrun --sdk macosx --show-sdk-path)"',
          'cmake .. -DWITH_PYBIND=ON -DWITH_TESTS=ON -G Ninja -DCMAKE_CXX_FLAGS=-fcolor-diagnostics -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER_LAUNCHER=ccache',
          'ninja -v check',
        ],
      },
    ],
  },
]
