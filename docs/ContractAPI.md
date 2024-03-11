#  init action
Contract must be initialized with init action before calling of any action

```bash
cleos push action profiles init '{"contract_name":"profiles","contract_version":"0.0.1","initial_admin":"profiles"}' -p profiles@active
```

# Create new profile action
Parameters display_name, avatar, bio are optional

```bash
cleos push action profiles newprofile '{"account":"profiles", "status": "status", "display_name": null, "avatar": null , "bio": null}'  -p profiles@active
```

# Tables 
Profiles table 
```bash
cleos get table profiles profiles profiles
```

# Config table
```bash
cleos get table profiles profiles config
```
