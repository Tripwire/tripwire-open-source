# These files change the behavior of the root account
(
  rulename = "Root config files",
  severity = 100
)
{
     /root                             -> $(SEC_CRIT) ; # Catch all additions to /root
     /root/.Xresources                 -> $(SEC_CONFIG) ;
     /root/.bashrc                     -> $(SEC_CONFIG) ;
     /root/.bash_profile               -> $(SEC_CONFIG) ;
     /root/.bash_logout                -> $(SEC_CONFIG) ;
     /root/.cshrc                      -> $(SEC_CONFIG) ;
     /root/.tcshrc                     -> $(SEC_CONFIG) ;
    #/root/Mail                        -> $(SEC_CONFIG) ;
    #/root/mail                        -> $(SEC_CONFIG) ;
    #/root/.amandahosts                -> $(SEC_CONFIG) ;
    #/root/.addressbook.lu             -> $(SEC_CONFIG) ;
    #/root/.addressbook                -> $(SEC_CONFIG) ;
     /root/.bash_history               -> $(SEC_CONFIG) ;
    #/root/.elm                        -> $(SEC_CONFIG) ;
     /root/.esd_auth                   -> $(SEC_CONFIG) ;
    #/root/.gnome_private              -> $(SEC_CONFIG) ;
    #/root/.gnome-desktop              -> $(SEC_CONFIG) ;
     /root/.gnome                      -> $(SEC_CONFIG) ;
     /root/.ICEauthority               -> $(SEC_CONFIG) ;
    #/root/.mc                         -> $(SEC_CONFIG) ;
    #/root/.pinerc                     -> $(SEC_CONFIG) ;
    #/root/.sawfish                    -> $(SEC_CONFIG) ;
     /root/.Xauthority                 -> $(SEC_CONFIG) -i ; # Changes Inode number on login
    #/root/.xauth                      -> $(SEC_CONFIG) ;
    #/root/.xsession-errors            -> $(SEC_CONFIG) ;
}

