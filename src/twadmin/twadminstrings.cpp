//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2019 Tripwire,
// Inc. Tripwire is a registered trademark of Tripwire, Inc.  All rights
// reserved.
//
// This program is free software.  The contents of this file are subject
// to the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.  You may redistribute it and/or modify it
// only in compliance with the GNU General Public License.
//
// This program is distributed in the hope that it will be useful.
// However, this program is distributed AS-IS WITHOUT ANY
// WARRANTY; INCLUDING THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS
// FOR A PARTICULAR PURPOSE.  Please see the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.
//
// Nothing in the GNU General Public License or any other license to use
// the code or files shall permit you to use Tripwire's trademarks,
// service marks, or other intellectual property without Tripwire's
// prior written consent.
//
// If you have any questions, please contact Tripwire, Inc. at either
// info@tripwire.org or www.tripwire.org.
//
//
// Name....: twadminstrings.cpp
// Date....: 05/11/99
// Creator.: Brian McFeely (bmcfeely)
//

#include "stdtwadmin.h"
#include "twadmin.h"
#include "twadminstrings.h"

#if IS_AROS
#    define VERSION_PREFIX "$VER: "
#else
#    define VERSION_PREFIX "@(#)"
#endif

TSS_BeginStringtable(cTWAdmin)

    TSS_StringEntry(twadmin::STR_TWADMIN_VERSION, _T("twadmin: Tripwire administrative tool.\n")),
    TSS_StringEntry(twadmin::STR_EMBEDDED_VERSION, _T(VERSION_PREFIX "twadmin " PACKAGE_VERSION)),

    TSS_StringEntry(twadmin::STR_TWADMIN_USAGE_SUMMARY,
                    _T("Usage:\n")
                    _T("\n")
                    _T("Create Configuration File: twadmin [-m F|--create-cfgfile][options] cfgfile.txt\n")
                    _T("Print Configuration File: twadmin [-m f|--print-cfgfile] [options]\n")
                    _T("Create Policy File: twadmin [-m P|--create-polfile] [options] polfile.txt\n")
                    _T("Print Policy File: twadmin [-m p|--print-polfile] [options]\n")
                    _T("Remove Encryption: twadmin [-m R|--remove-encryption] [options] [file1...]\n")
                    _T("Encryption: twadmin [-m E|--encrypt] [options] [file1...]\n")
                    _T("Examine Encryption: twadmin [-m e|--examine] [options] [file1...]\n")
                    _T("Generate Keys: twadmin [-m G|--generate-keys] [options]\n")
                    _T("Change Passphrases: twadmin [-m C|--change-passphrases] [options]\n")
                    _T("\n")
                    _T("Type 'twadmin [mode] --help' OR\n")
                    _T("'twadmin --help mode [mode...]' OR\n")
                    _T("'twadmin --help all' for extended help\n")),

    TSS_StringEntry(twadmin::STR_TWADMIN_HELP_CREATE_CFGFILE,
                    _T("Create Configuration File mode:\n")
                    _T("  -m F                 --create-cfgfile\n")
                    _T("  -v                   --verbose\n")
                    _T("  -s                   --silent, --quiet\n")
                    _T("  -c cfgfile           --cfgfile cfgfile\n")
                    _T("  -S sitekey           --site-keyfile sitekey\n")
                    _T("  -Q passphrase        --site-passphrase passphrase\n")
                    _T("  -e                   --no-encryption\n")
                    _T("configfile.txt\n")
                    _T("\n")
                    _T("The -v and -s options are mutually exclusive.\n")
                    _T("Exactly one of -e or -S must be specified.\n")
                    _T("The -Q option is valid only with -S.\n")
                    _T("\n")),

    TSS_StringEntry(twadmin::STR_TWADMIN_HELP_PRINT_CFGFILE,
                    _T("Print Configuration File mode:\n")
                    _T("  -m f                 --print-cfgfile\n")
                    _T("  -v                   --verbose\n")
                    _T("  -s                   --silent, --quiet\n")
                    _T("  -c cfgfile           --cfgfile cfgfile\n")
                    _T("\n")
                    _T("The -v and -s options are mutually exclusive.\n")
                    _T("\n")),

    TSS_StringEntry(twadmin::STR_TWADMIN_HELP_CREATE_POLFILE,
                    _T("Replace Policy File mode:\n")
                    _T("  -m P                 --create-polfile\n")
                    _T("  -v                   --verbose\n")
                    _T("  -s                   --silent, --quiet\n")
                    _T("  -c cfgfile           --cfgfile cfgfile\n")
                    _T("  -p polfile           --polfile polfile\n")
                    _T("  -S sitekey           --site-keyfile sitekey\n")
                    _T("  -Q passphrase        --site-passphrase passphrase\n")
                    _T("  -e                   --no-encryption\n")
                    _T("policyfile.txt\n")
                    _T("\n")
                    _T("The -v and -s options are mutually exclusive.\n")
                    _T("The -e and -S options are mutually exclusive.\n")
                    _T("The -e and -Q options are mutually exclusive.\n")
                    _T("\n")),

    TSS_StringEntry(twadmin::STR_TWADMIN_HELP_PRINT_POLFILE,
                    _T("Print Policy File mode:\n")
                    _T("  -m p                 --print-polfile\n")
                    _T("  -v                   --verbose\n")
                    _T("  -s                   --silent, --quiet\n")
                    _T("  -c cfgfile           --cfgfile cfgfile\n")
                    _T("  -p polfile           --polfile polfile\n")
                    _T("  -S sitekey           --site-keyfile sitekey\n")
                    _T("\n")
                    _T("The -v and -s options are mutually exclusive.\n")
                    _T("\n")),

    TSS_StringEntry(twadmin::STR_TWADMIN_HELP_REMOVE_ENCRYPTION,
                    _T("Remove Encryption mode:\n")
                    _T("  -m R                 --remove-encryption\n")
                    _T("  -v                   --verbose\n")
                    _T("  -s                   --silent, --quiet\n")
                    _T("  -c cfgfile           --cfgfile cfgfile\n")
                    _T("  -L localkey          --local-keyfile localkey\n")
                    _T("  -S sitekey           --site-keyfile sitekey\n")
                    _T("  -P passphrase        --local-passphrase passphrase\n")
                    _T("  -Q passphrase        --site-passphrase passphrase\n")
                    _T("file1 [file2 ...]\n")
                    _T("\n")
                    _T("The -v and -s options are mutually exclusive.\n")
                    _T("\n")),

    TSS_StringEntry(twadmin::STR_TWADMIN_HELP_ENCRYPT,
                    _T("Encryption mode:\n")
                    _T("  -m E                 --encrypt\n")
                    _T("  -v                   --verbose\n")
                    _T("  -s                   --silent, --quiet\n")
                    _T("  -c cfgfile           --cfgfile cfgfile\n")
                    _T("  -L localkey          --local-keyfile localkey\n")
                    _T("  -S sitekey           --site-keyfile sitekey\n")
                    _T("  -P passphrase        --local-passphrase passphrase\n")
                    _T("  -Q passphrase        --site-passphrase passphrase\n")
                    _T("file1 [file2 ...]\n")
                    _T("\n")
                    _T("The -v and -s options are mutually exclusive.\n")
                    _T("\n")),

    TSS_StringEntry(twadmin::STR_TWADMIN_HELP_EXAMINE,
                    _T("Examine Encryption mode:\n")
                    _T("  -m e                 --examine\n")
                    _T("  -v                   --verbose\n")
                    _T("  -s                   --silent, --quiet\n")
                    _T("  -c cfgfile           --cfgfile cfgfile\n")
                    _T("  -L localkey          --local-keyfile localkey\n")
                    _T("  -S sitekey           --site-keyfile sitekey\n")
                    _T("file1 [file2 ...]\n")
                    _T("\n")
                    _T("The -v and -s options are mutually exclusive.\n")
                    _T("\n")),

    TSS_StringEntry(twadmin::STR_TWADMIN_HELP_GENERATE_KEYS,
                    _T("Generate Keys mode:\n")
                    _T("  -m G                 --generate-keys\n")
                    _T("  -v                   --verbose\n")
                    _T("  -s                   --silent, --quiet\n")
                    _T("  -L localkey          --local-keyfile localkey\n")
                    _T("  -S sitekey           --site-keyfile sitekey\n")
                    _T("  -P passphrase        --local-passphrase passphrase\n")
                    _T("  -Q passphrase        --site-passphrase passphrase\n")
                    _T("  -K size              --key-size size [1024 or 2048]\n")
                    _T("\n")
                    _T("The -v and -s options are mutually exclusive.\n")
                    _T("Exactly one of -S or -L must be specified.\n")
                    _T("\n")),

    TSS_StringEntry(twadmin::STR_TWADMIN_HELP_CHANGE_PASSPHRASES,
                    _T("Change Passphrases mode:\n")
                    _T("  -m C                 --change-passphrases\n")
                    _T("  -v                   --verbose\n")
                    _T("  -s                   --silent, --quiet\n")
                    _T("  -L localkey          --local-keyfile localkey\n")
                    _T("  -S sitekey           --site-keyfile sitekey\n")
                    _T("  -P passphrase        --local-passphrase passphrase\n")
                    _T("  -Q passphrase        --site-passphrase passphrase\n")
                    _T("                       --local-passphrase-old passphrase-old\n")
                    _T("                       --site-passphrase-old passphrase-old\n")
                    _T("\n")
                    _T("The -v and -s options are mutually exclusive.\n")
                    _T("At least one of -S or -L must be specified.\n")
                    _T("\n")),

    TSS_StringEntry(twadmin::STR_KEYGEN_VERBOSE_OUTPUT_FILES,
                    _T("Using site keyfile: \"%s\" and local keyfile: \"%s\"\n")),
    TSS_StringEntry(twadmin::STR_KEYGEN_VERBOSE_PASSPHRASES, _T("Using supplied passphrases.\n")),
    TSS_StringEntry(twadmin::STR_KEYGEN_VERBOSE_SITEKEY, _T("Generating site key: %s\n")),
    TSS_StringEntry(twadmin::STR_KEYGEN_VERBOSE_LOCALKEY, _T("Generating local key: %s\n")),
    TSS_StringEntry(twadmin::STR_UPCONFIG_VERBOSE_PT_CONFIG, _T("Using plaintext config file: %s\n")),
    TSS_StringEntry(twadmin::STR_UPCONFIG_CREATING_CONFIG, _T("Writing configuration file: %s\n")),
    TSS_StringEntry(twadmin::STR_UPCONFIG_VERBOSE_PT_POLICY, _T("Using plaintext policy file: %s\n")),
    TSS_StringEntry(twadmin::STR_GENERATING_KEYS, _T("Generating key (this may take several minutes)...")),
    TSS_StringEntry(twadmin::STR_GENERATION_COMPLETE, _T("Key generation complete.\n")),
    TSS_StringEntry(twadmin::STR_SITEKEYFILE, _T("\tSite Keyfile: ")),
    TSS_StringEntry(twadmin::STR_LOCALKEYFILE, _T("\tLocal Keyfile: ")),
    TSS_StringEntry(twadmin::STR_SITEKEY_EXISTS_1, _T("The site key file: \"")),
    TSS_StringEntry(twadmin::STR_SITEKEY_EXISTS_2, _T("\" exists. Overwrite (Y/N)? ")),
    TSS_StringEntry(twadmin::STR_LOCALKEY_EXISTS_1, _T("The local key file: \"")),
    TSS_StringEntry(twadmin::STR_LOCALKEY_EXISTS_2, _T("\" exists. Overwrite (Y/N)? ")),
    TSS_StringEntry(twadmin::STR_KEYFILE_BACKED_UP_AS, _T("Keyfile backed up as %s\n")),
    TSS_StringEntry(twadmin::STR_CONVERTING_FILES, _T("Converting files.\n")),
    TSS_StringEntry(twadmin::STR_EXAMINING_FILE, _T("Examining file: ")),
    TSS_StringEntry(twadmin::STR_KEYS_DECRYPT, _T("The following keys decrypt this file: ")),
    TSS_StringEntry(twadmin::STR_BACKUP_EXISTS_1, _T("Backup file ")),
    TSS_StringEntry(twadmin::STR_BACKUP_EXISTS_2, _T(" exists. Overwrite (Y/N)? ")),
    TSS_StringEntry(twadmin::STR_PASSPHRASE_HINT,
                    _T("\n(When selecting a passphrase, keep in mind that good passphrases typically\n")
                    _T("have upper and lower case letters, digits and punctuation marks, and are\n")
                    _T("at least 8 characters in length.)\n\n")),
    TSS_StringEntry(twadmin::STR_POL_NOT_UPDATED, _T("The policy file was not altered.\n")),
    TSS_StringEntry(twadmin::STR_ENCRYPT_TYPE_NONE, _T("Encoding: None\n")),
    TSS_StringEntry(twadmin::STR_ENCRYPT_TYPE_COMP, _T("Encoding: Compressed\n")),
    TSS_StringEntry(twadmin::STR_ENCRYPT_TYPE_ASYM, _T("Encoding: Asymmetric Encryption\n")),
    TSS_StringEntry(twadmin::STR_ENCRYPT_TYPE_UNK, _T("Encoding: Unrecognized\n")),
    TSS_StringEntry(twadmin::STR_FILE_TYPE_DB, _T("File Type: Tripwire Database (Ver %X.%X.%X.%X)\n")),
    TSS_StringEntry(twadmin::STR_FILE_TYPE_REP, _T("File Type: Tripwire Report (Ver %X.%X.%X.%X)\n")),
    TSS_StringEntry(twadmin::STR_FILE_TYPE_CFG, _T("File Type: Tripwire Config File (Ver %X.%X.%X.%X)\n")),
    TSS_StringEntry(twadmin::STR_FILE_TYPE_POL, _T("File Type: Tripwire Policy File (Ver %X.%X.%X.%X)\n")),
    TSS_StringEntry(twadmin::STR_FILE_TYPE_KEY, _T("File Type: Tripwire Key File (Ver %X.%X.%X.%X)\n")),
    TSS_StringEntry(twadmin::STR_FILE_TYPE_UNK, _T("File Type: Unknown\n")),
    TSS_StringEntry(twadmin::STR_ENTER_SITE_PASS, _T("Enter the site keyfile passphrase:")),
    TSS_StringEntry(twadmin::STR_VERIFY_SITE_PASS, _T("Verify the site keyfile passphrase:")),
    TSS_StringEntry(twadmin::STR_ENTER_LOCAL_PASS, _T("Enter the local keyfile passphrase:")),
    TSS_StringEntry(twadmin::STR_VERIFY_LOCAL_PASS, _T("Verify the local keyfile passphrase:")),
    TSS_StringEntry(twadmin::STR_ENTER_SITE_PASS_OLD, _T("Enter the old site keyfile passphrase:")),
    TSS_StringEntry(twadmin::STR_ENTER_LOCAL_PASS_OLD, _T("Enter the old local keyfile passphrase:")),
    TSS_StringEntry(twadmin::STR_REMOVE_ENCRYPTION_WARNING,
                    _T("NOTE: Removing encryption on a file leaves it open to tampering!\n")),
    TSS_StringEntry(twadmin::STR_ENCRYPTION_REMOVED, _T("Encryption removed from \"%s\" successfully.\n")),
    TSS_StringEntry(twadmin::STR_ENCRYPTION_SUCCEEDED, _T("\"%s\" encrypted successfully.\n")),
    TSS_StringEntry(twadmin::STR_FILE, _T("File: \"")), TSS_StringEntry(twadmin::STR_ENDQUOTE_NEWLINE, _T("\"\n")),

    TSS_StringEntry(twadmin::STR_ERR2_NO_PT_CONFIG, _T("No plaintext config file specified.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_NO_CONFIG, _T("No config file specified.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_NO_PT_POLICY, _T("No plaintext policy file specified.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_NO_POLICY, _T("No policy file specified.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_CONFIG_OPEN, _T("Config file could not be opened.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_SITE_KEY_NOENCRYPT_NOT_SPECIFIED,
                    _T("Site key file or no-encryption must be specified.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_LOCAL_KEY_NOT_SPECIFIED, _T("Local key file must be specified.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_KEYS_NOT_SPECIFIED, _T("Site or local key file must be specified.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_KEY_FILENAMES_IDENTICAL,
                    _T("Site and local key filenames may not be identical.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_SITE_KEY_DOESNT_EXIST,
                    _T("Site key does not exist.  Use -m G mode to generate a new key.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_SITE_KEY_READ_ONLY, _T("Site key is read only, cannot overwrite.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_LOCAL_KEY_DOESNT_EXIST,
                    _T("Local key does not exist.  Use -m G mode to generate a new key.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_LOCAL_KEY_READ_ONLY, _T("Local key is read only, cannot overwrite.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_COULDNT_RENAME_FILE, _T("File could not be renamed ")),
    TSS_StringEntry(twadmin::STR_ERR2_CONVERSION_FILE_READ_ONLY1, _T("File to be converted ")),
    TSS_StringEntry(twadmin::STR_ERR2_CONVERSION_FILE_READ_ONLY2, _T(" is Read Only, key change aborted.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_UNABLE_TO_PRINT_POLICY, _T("\nUnable to print policy file.")),
    TSS_StringEntry(twadmin::STR_ERR2_CAN_NOT_ENCRYPT_KEYFILE, _T("Can not encrypt a keyfile.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_CAN_NOT_DECRYPT_KEYFILE, _T("Can not remove encryption on a keyfile.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_NO_FILES_SPECIFIED, _T("No files specified.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_LONE_SITE_PASSPHRASE,
                    _T("Site passphrase was specified without corresponding site keyfile.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_LONE_LOCAL_PASSPHRASE,
                    _T("Local passphrase was specified without corresponding local keyfile.\n")),

    TSS_StringEntry(twadmin::STR_ERR2_FILE_DOES_NOT_EXIST, _T("File does not exist.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_FILE_COULD_NOT_BE_OPENED, _T("File could not be opened.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_FILE_COULD_NOT_BE_READ, _T("File could not be read.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_FILE_NOT_A_TW_FILE, _T("File is not a Tripwire data file.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_FILE_COULD_NOT_BE_EXAMINED, _T("File could not be examined.\n")),

    //TSS_StringEntry( twadmin::STR_ERR2_FILE_TYPE_UNKNOWN,         _T("Error: File type is unknown for file %s\n") ),
    TSS_StringEntry(twadmin::STR_ERR2_ENCODING_TYPE_UNKNOWN, _T("Encoding type is unknown.")),
    TSS_StringEntry(twadmin::STR_ERR2_FILE_NOT_ENCRYPED,
                    _T("This file is not encrypted, decryption skipped.\nFilename: ")),
    TSS_StringEntry(twadmin::STR_ERR2_REMOVE_ENCRYPTION_FAILED, _T("Encryption removal failed.\nFilename: ")),
    TSS_StringEntry(twadmin::STR_ERR2_COULD_NOT_OPEN_PROVIDED_KEYFILE,
                    _T("Error: Provided keyfile could not be opened ")),
    TSS_StringEntry(twadmin::STR_ERR2_FILE_ALREADY_ENCRYPTED, _T("File is currently encrypted.  Skipping.\n")),
    TSS_StringEntry(twadmin::STR_ERR2_ENCRYPTION_FAILED, _T("Encryption failed.\n")),

    // keygeneration
    TSS_StringEntry(twadmin::STR_ERR2_KEYGEN_FILEWRITE, _T("Error: File could not be written to: ")),
    TSS_StringEntry(twadmin::STR_ERR2_KEYGEN, _T("Error generating key, ")),
    TSS_StringEntry(twadmin::STR_ERR2_KEYGEN2, _T(" not written.")),
    TSS_StringEntry(twadmin::STR_ERR2_PASSPHRASE_NOKEY,
                    _T("Error: A passphrase has been specified without the corresponding key.")),

    TSS_StringEntry(twadmin::STR_ERR2_CREATE_CFG_MISSING_KEYFILE,
                    _T("A config file can not be created and encrypted with a keyfile unless the same keyfile is ")
                    _T("specified as the SITEKEYFILE within the new config file text.")),
    TSS_StringEntry(twadmin::STR_ERR2_CREATE_CFG_SITEKEY_MISMATCH1, _T("The specified keyfile \"")),
    TSS_StringEntry(twadmin::STR_ERR2_CREATE_CFG_SITEKEY_MISMATCH2,
                    _T("\" does not match the keyfile specified in the new config file text \"")),
    TSS_StringEntry(twadmin::STR_ERR2_CREATE_CFG_SITEKEY_MISMATCH3, _T("\".")),

    TSS_StringEntry(twadmin::STR_ERR2_INVALID_KEY_SIZE,
                    _T("Invalid key size specified. Valid sizes are 1024 & 2048 bits.")),

    TSS_EndStringtable(cTWAdmin)
