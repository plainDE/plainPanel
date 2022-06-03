<div align="center">
  <img src="readme-icon.png" width="96">
  <h1>plainPanel</h1>
  <p>Lightweight panel for OpenBox, FluxBox and other window managers.</p>
 
  <img src="https://img.shields.io/github/last-commit/plainDE/plainPanel?style=plastic">
  <img src="https://img.shields.io/github/license/plainDE/plainPanel?style=plastic">
  <img src="https://img.shields.io/github/issues/plainDE/plainPanel?style=plastic">
  
  <h2>Applets</h2>
  <img src="https://raw.githubusercontent.com/linuxmint/mint-y-icons/master/usr/share/icons/Mint-Y/apps/16/app-launcher.png"> App Menu<br>
  <img src="https://raw.githubusercontent.com/linuxmint/mint-y-icons/master/usr/share/icons/Mint-Y/apps/16/preferences-system-windows.png"> Window List<br>
  <img src="https://raw.githubusercontent.com/linuxmint/mint-y-icons/master/usr/share/icons/Mint-Y/apps/16/extensions.png"> Spacer<br>
  <img src="https://raw.githubusercontent.com/linuxmint/mint-y-icons/master/usr/share/icons/Mint-Y/apps/16/workspace-switcher-top-left.png"> Worskpaces<br>
  <img src="https://raw.githubusercontent.com/linuxmint/mint-y-icons/master/usr/share/icons/Mint-Y/apps/16/preferences-system-sound.png"> Volume<br>
  <img src="https://raw.githubusercontent.com/linuxmint/mint-y-icons/master/usr/share/icons/Mint-Y/apps/16/keyboard.png"> Keyboard Layout<br>
  <img src="https://raw.githubusercontent.com/linuxmint/mint-y-icons/master/usr/share/icons/Mint-Y/apps/16/calendar.png"> Date & Time<br>
  <img src="https://raw.githubusercontent.com/linuxmint/mint-y-icons/master/usr/share/icons/Mint-Y/apps/16/extensions.png"> Splitter<br>
  <img src="https://raw.githubusercontent.com/linuxmint/mint-y-icons/master/usr/share/icons/Mint-Y/apps/16/preferences-desktop-user.png"> User menu<br>
  
  <h2>Screenshots</h2>
  <img src="scr-0.1.4.png" width=640 height=480>
  
  <h2>Dependencies</h2>
  <code>qt6-base</code>, <code>noto-fonts-emoji</code>, <code>polkit</code>, <code>ttf-opensans</code>, <code>make</code>, <code>alsa-utils</code>, <code>kwindowsystem</code>, <code>python3</code>, <code>xcompmgr</code>
  
  <h2>Installation</h2>
  You can either use <a href="https://github.com/plainDE/plainInstaller">plainInstaller</a>, a script that will download, compile and install everything automatically, or install plainPanel only.<br><br>
  
  <div align="left">
    1. <code>git clone https://github.com/plainDE/plainPanel && cd plainPanel</code><br>
    2. <code>qmake</code><br>
    3. <code>make</code><br>
    4. <code>sudo mkdir /usr/share/plainDE && sudo cp ./plainPanel /usr/share/plainDE/</code><br>
    5. <code>sudo cp readme-icon.png /usr/share/plainDE/menuIcon.png</code>
  </div><br>
  
  Now add <code>plainPanel</code> to OpenBox/FluxBox/... autostart and enjoy!
  
  <h2>Customizing</h2>
  You can either edit ~/.config/plainDE/config.json manually or use <a href="https://github.com/plainDE/plainControlCenter">plainControlCenter</a>.
  
  <h2>How can I help you?</h2>
  <ul>
    <li>Create a new applet</li>
    <li>Translate plainPanel into another language (soon)</li>
    <li>Fix a bug or suggest solution for it</li>
    <li>Suggest an interesting idea</li>
    <li>...</li>
  </ul>
  
  Any help is appreciated.
  Email us at <a href="mailto:segfault@plainde.org">segfault@plainde.org</a>
  
  <h2>Miscellaneous</h2>
  <b>Note</b>. Mint-Y is recommended icon theme (we use few Mint-Y-only icons).<br>
  <b>Note</b>. Use setxkbmap to change your keyboard layout.
  
</div>
