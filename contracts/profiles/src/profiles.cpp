#include <profiles.hpp>

const string error_must_init_message = "contract must be initialized. call init action";
const string profile_not_found_message = "profile not found";

// ACTION profiles::delconf()
// {
//     config_singleton configs(get_self(), get_self().value);
//     auto conf = configs.get();
//     require_auth(conf.admin);
//     configs.remove();
// }

//======================== config actions ========================

ACTION profiles::init(const string& contract_name, const string& contract_version, const name& initial_admin)
{
    //authenticate
    require_auth(get_self());

    //open config table
    config_singleton configs(get_self(), get_self().value);

    //validate
    check(!configs.exists(), "contract already initialized");
    check(is_account(initial_admin), "initial admin account does not exist");

    //build new config
    config new_config = {
        contract_name,
        contract_version,
        initial_admin
    };

    //set new config
    //ram payer: contract
    configs.set(new_config, get_self());
}

ACTION profiles::setversion(const string& new_version)
{
    //open configs singleton
    config_singleton configs(get_self(), get_self().value);
    check(configs.exists(), error_must_init_message.c_str());
    auto conf = configs.get();

    //authenticate
    require_auth(conf.admin);

    //update contract version
    conf.contract_version = new_version;

    //set config
    configs.set(conf, get_self());
}

ACTION profiles::setadmin(const name& new_admin)
{
    //open configs singleton
    config_singleton configs(get_self(), get_self().value);
    check(configs.exists(), error_must_init_message.c_str());
    auto conf = configs.get();

    //authenticate
    require_auth(conf.admin);

    //update admin
    conf.admin = new_admin;

    //set config
    configs.set(conf, get_self());
}

ACTION profiles::setdefavatar(const string& new_default_avatar)
{
    //open configs singleton
    config_singleton configs(get_self(), get_self().value);
    check(configs.exists(), error_must_init_message.c_str());
    auto conf = configs.get();

    //authenticate
    require_auth(conf.admin);

    //update default avatar
    conf.default_avatar = new_default_avatar;

    //set config
    configs.set(conf, get_self());
}

ACTION profiles::setlength(const name& length_name, uint32_t new_length)
{
    //open configs singleton
    config_singleton configs(get_self(), get_self().value);
    check(configs.exists(), error_must_init_message.c_str());
    auto conf = configs.get();

    //authenticate
    require_auth(conf.admin);

    //match length setting by name
    switch (length_name.value)
    {
        case name("displayname").value :
            //update max display name length
            conf.max_display_name_length = new_length;
            break;
        case name("avatar").value :
            //update max avatar length
            conf.max_avatar_length = new_length;
            break;
        case name("bio").value :
            //update max bio length
            conf.max_bio_length = new_length;
        case name("status").value :
            //update max status length
            conf.max_status_length = new_length;
            break;
        default:
            check(false, "invalid length name");
            break;
    }

    //set config
    configs.set(conf, get_self());
}

//======================== profile actions ========================

ACTION profiles::newprofile(const name& account, const string& status, optional<string> display_name, optional<string> avatar, optional<string> bio)
{
    //authenticate
    require_auth(account);

    //open profiles table, find profile
    profiles_table profs(get_self(), get_self().value);
    auto prof_itr = profs.find(account.value);

    //validate
    check(prof_itr == profs.end(), "profile: " + account.to_string() + " already exists");

    //open configs singleton, get config
    config_singleton configs(get_self(), get_self().value);
    check(configs.exists(), error_must_init_message.c_str());
    auto conf = configs.get();

    //validate status
    check_size(status, conf.max_status_length, "status");

    //initialize
    const string profile_display_name = (display_name) ? *display_name : account.to_string();
    const string profile_avatar = (avatar) ? *avatar : conf.default_avatar;
    const string profile_bio = (bio) ? *bio : string("");

    //validate
    check_size(profile_display_name, conf.max_display_name_length, "display");
    check_size(profile_avatar, conf.max_avatar_length, "avatar image link");
    check_size(profile_bio, conf.max_bio_length, "bio");
    
    //emplace new profile
    //ram payer: contract
    profs.emplace(get_self(), [&](auto& col) {
        col.status = status;
        col.account_name = account;
        col.display_name = profile_display_name;
        col.avatar = profile_avatar;
        col.bio = profile_bio;
    });
}

ACTION profiles::editdisplay(const name& account, const string& new_display_name)
{
    //authenticate
    require_auth(account);

    //open profiles table, get profile
    profiles_table profs(get_self(), get_self().value);
    auto& prof = profs.get(account.value, profile_not_found_message.c_str());

    //open configs singleton, get config
    config_singleton configs(get_self(), get_self().value);
    check(configs.exists(), error_must_init_message.c_str());
    auto conf = configs.get();

    //validate
    check_size(new_display_name, conf.max_display_name_length, "display name");

    //update profile
    profs.modify(prof, same_payer, [&](auto& col) {
        col.display_name = new_display_name;
    });
}

ACTION profiles::editavatar(const name& account, const string& new_avatar)
{
    //authenticate
    require_auth(account);

    //open profiles table, get profile
    profiles_table profs(get_self(), get_self().value);
    auto& prof = profs.get(account.value, profile_not_found_message.c_str());

    //open configs singleton, get config
    config_singleton configs(get_self(), get_self().value);
    check(configs.exists(), error_must_init_message.c_str());
    auto conf = configs.get();

    //validate
    check_size(new_avatar, conf.max_avatar_length, "avatar image link");

    //update profile
    profs.modify(prof, same_payer, [&](auto& col) {
        col.avatar = new_avatar;
    });
}

ACTION profiles::editbio(const name& account, const string& new_bio)
{
    //authenticate
    require_auth(account);

    //open profiles table, get profile
    profiles_table profs(get_self(), get_self().value);
    auto& prof = profs.get(account.value, profile_not_found_message.c_str());

    //open configs singleton, get config
    config_singleton configs(get_self(), get_self().value);
    check(configs.exists(), error_must_init_message.c_str());
    auto conf = configs.get();

    //validate
    check_size(new_bio, conf.max_bio_length, "bio");

    //update profile
    profs.modify(prof, same_payer, [&](auto& col) {
        col.bio = new_bio;
    });
}

ACTION profiles::editstatus(const name& account, const string& new_status)
{
    //authenticate
    require_auth(account);

    //open profiles table, get profile
    profiles_table profs(get_self(), get_self().value);
    auto& prof = profs.get(account.value, profile_not_found_message.c_str());

    //open configs singleton, get config
    config_singleton configs(get_self(), get_self().value);
    check(configs.exists(), error_must_init_message.c_str());
    auto conf = configs.get();

    //validate
    check_size(new_status, conf.max_status_length, "status");

    //update profile
    profs.modify(prof, same_payer, [&](auto& col) {
        col.status = new_status;
    });
}

ACTION profiles::verify(const name& account)
{
    //authenticate
    require_auth(permission_level{get_self(), name("verify")});

    //open profiles table, get profile
    profiles_table profs(get_self(), get_self().value);
    auto& prof = profs.get(account.value, profile_not_found_message.c_str());

    //update profile
    profs.modify(prof, same_payer, [&](auto& col) {
        col.is_verified = true;
    });
}

ACTION profiles::delprofile(const name& account)
{
    //authenticate
    require_auth(account);

    //open profiles table, get profile
    profiles_table profs(get_self(), get_self().value);
    auto& prof = profs.get(account.value, profile_not_found_message.c_str());

    //erase profile
    profs.erase(prof);
}

//======================== metadata actions ========================

ACTION profiles::writemeta(const name& writer, const name& account, const string& data)
{
    //authenticate
    require_auth(writer);

    //TODO: validate data length

    //open profiles table, get profile
    profiles_table profiles(get_self(), get_self().value);
    auto& prof = profiles.get(account.value, profile_not_found_message.c_str());

    //open metadata table, find meta
    metadata_table metadata(get_self(), writer.value);
    auto meta_itr = metadata.find(account.value);

    //if metadata not found
    if (meta_itr == metadata.end()) {
        //emplace new metadata
        //ram payer: writer
        metadata.emplace(writer, [&](auto& col) {
            col.account = account;
            col.data = data;
        });
    } else {
        //overwrite existing metadata
        metadata.modify(*meta_itr, same_payer, [&](auto& col) {
            col.data = data;
        });
    }
}

ACTION profiles::delmeta(const name& writer, const name& account)
{
    //authenticate
    require_auth(writer);

    //open metadata table, find meta
    metadata_table metadata(get_self(), writer.value);
    auto& meta = metadata.get(account.value, "metadata not found");

    //delete metadata
    metadata.erase(meta);
}

void profiles::check_size(const string& parameter_value, uint32_t max_allowed_size, const string& parameter_name)
{
    //validate size
    check(parameter_value.length() <= max_allowed_size, parameter_name + " too long. Maximum allowed is "
        + to_string(max_allowed_size) + " , you entered size " + to_string(parameter_value.length()));
}