# Maintainer: Your Name <you@example.com>

pkgname=hypr-control-center
pkgver=0.1.0
pkgrel=1
pkgdesc="Hyprland settings manager — GNOME Control Center fork ported to Hyprland"
arch=('x86_64')
url="https://github.com/anomalyco/hypr-control-center"
license=('GPL2+')
depends=(
  'gtk4' 'libadwaita' 'gdk-pixbuf2' 'glib2' 'libxml2' 'libpulse'
  'upower' 'libgudev' 'libepoxy' 'json-glib' 'gnome-online-accounts'
  'colord' 'gsettings-desktop-schemas' 'polkit' 'cups' 'libwacom'
  'fontconfig' 'gcr-4' 'accountsservice' 'ibus' 'libgtop' 'cairo'
  'networkmanager' 'libnma-gtk4' 'modemmanager'
)
makedepends=(
  'meson' 'ninja' 'blueprint-compiler' 'python' 'gettext'
  'glib2' 'gtk4'
)
optdepends=(
  'smbclient: printer browsing'
)
conflicts=('gnome-control-center')
install="${pkgname}.install"
provides=("${pkgname}=${pkgver}")
source=("${pkgname}-${pkgver}::git+file://${PWD}")
sha256sums=('SKIP')

build() {
  arch-meson "${srcdir}/${pkgname}-${pkgver}" build \
    -Dtests=false \
    -Ddocumentation=false
  ninja -C build
}

package() {
  DESTDIR="${pkgdir}" ninja -C build install
}
