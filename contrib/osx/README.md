# OSX readme

## What this does

- Runs tripwire whenever connected to power
- Automatically grooms logs (not reports, its a todo)


## Installation

1. Edit `./homebrew_install_scripts_and_config` to taste
2. Run `./homebrew_install_scripts_and_config` as root
3. Modify `TRIPWIRE_ETC_DIR/twcfg.txt` and `TRIPWIRE_ETC_DIR/twpol.txt` to taste
4. Make up two brand-new, 24+ character passwords (they MUST not be the same), and store them securely before continuing:

  - site passphrase
  - local passphrase

5. Run the following commands as root:

    ```
    TRIPWIRE_ROOT/sbin/tripwire_set_site_and_local_passphrases
    TRIPWIRE_ROOT/sbin/tripwire_update_config_file # twcfg.txt -> tw.cfg
    TRIPWIRE_ROOT/sbin/tripwire_update_policy_file # twpol.txt -> tw.pol
    TRIPWIRE_ROOT/sbin/tripwire --init             # this creates a new encrypted database
    ```

6. Enable periodic jobs (run as root): 

    ```
    launchctl load -w /Library/LaunchDaemons/org.tripwire.Tripwire.plist
    ```


## Uninstallation (run with sudo or su)

    launchctl unload -w /Library/LaunchDaemons/org.tripwire.Tripwire.plist
    rm -f /Library/LaunchDaemons/org.tripwire.Tripwire.plist
    TRIPWIRE_ROOT/sbin/tripwire_uninstall
    # or remove everything: TRIPWIRE_ROOT/sbin/tripwire_uninstall -A 

