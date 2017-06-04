# Registry

Registry servers have no preferred language but must contain the following
endpoints to be valid.

The default registry URL is `http://localhost:8000`.

Note: To change the registry URL, run:

```text
nessemble registry <URL>
```

## Endpoints

| Endpoint                                                                  | Method   | Description                                   | Needs Auth                              |
|---------------------------------------------------------------------------|----------|-----------------------------------------------|-----------------------------------------|
| [/](#get)                                                                 | GET      | List of all packages                          | <i class="fa fa-times color-red"></i>   |
| [/search/&lt;string:term&gt;](#get-searchstringterm)                      | GET      | Search for packages by `term`                 | <i class="fa fa-times color-red"></i>   |
| [/package/&lt;string:package&gt;](#get-packagestringpackage)              | GET      | Display information about package `package`   | <i class="fa fa-times color-red"></i>   |
| [/package/&lt;string:package&gt;/README](#get-packagestringpackagereadme) | GET      | Display README for package `package`          | <i class="fa fa-times color-red"></i>   |
| [/package/&lt;string:package&gt;/data](#get-packagestringpackagedata)     | GET      | A tarball containing all data for `package`   | <i class="fa fa-times color-red"></i>   |
| [/package/publish](#post-packagepublish)                                  | POST     | Publish a new package                         | <i class="fa fa-check color-green"></i> |
| [/user/create](#post-usercreate)                                          | POST     | Create a user in the registry                 | <i class="fa fa-times color-red"></i>   |
| [/user/login](#post-userlogin)                                            | POST     | Log into the registry                         | <i class="fa fa-times color-red"></i>   |
| [/user/logout](#post-userlogout)                                          | GET/POST | Log out of the registry                       | <i class="fa fa-check color-green"></i> |
| [/user/forgotpassword](#post-userforgotpassword)                          | POST     | Send an email to reset password               | <i class="fa fa-times color-red"></i>   |
| [/user/resetpassword](#post-userresetpassword)                            | POST     | Reset password                                | <i class="fa fa-check color-green"></i> |
| [/reference](#get-reference)                                              | GET      | Display reference categories                  | <i class="fa fa-times color-red"></i>   |
| [/reference/<path:path>](#get-reference_1)                                | GET      | Display reference information for term `path` | <i class="fa fa-times color-red"></i>   |

### Root

#### GET /

List of all packages.

Request:

```text
GET / HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: application/json
```

Response:

```text
HTTP/1.1 200 OK
Content-Length: xxx
Content-Type: application/json
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

{
    "packages": [
        {
            "title": "foo",
            "description": "Foo bar baz qux",
            "tags": [
                "foo",
                "bar",
                "baz"
            ],
            "url": "http://xxxxx.xxx/foo"
        },
        ...
    ]
}
```

### /search

#### GET /search/&lt;string:term&gt;

Search for packages by `term`.

Request:

```text
GET /search/foo HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: application/json
```

Response:

```text
HTTP/1.1 200 OK
Content-Length: xxx
Content-Type: application/json
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

{
    "term": "foo",
    "results": [
        {
            "title": "foo",
            "description": "Foo bar baz qux",
            "tags": [
                "foo",
                "bar",
                "baz"
            ],
            "url": "http://xxxxx.xxx/foo"
        },
        ...
    ]
}
```

### /package

Note: The following endpoints also work with `/package/<string:version>`
in which `version` is a valid, specific version of the package.

#### GET /package/&lt;string:package&gt;

Display information about package `package`.

Request:

```text
GET /package/foo HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: application/json
```

Response:

```text
HTTP/1.1 200 OK
Content-Length: xxx
Content-Type: application/json
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

{
    "title": "foo",
    "description": "Foo bar baz qux",
    "version": "1.0.1",
    "version": {
        "1.0.1": "1997-08-29T22:14:00.000Z"
    },
    "author": {
        "name": "Joe Somebody",
        "email": "joe.somebody@email.com"
    },
    "license": "GPLv3",
    "tags": [
        "foo",
        "bar",
        "baz"
    ],
    "resource": "http://xxxxx.xxx/foo/data",
    "readme": "http://xxxxx.xxx/foo/README",
    "shasum": "0123456789abcdef0123456789abcdef12345678"
}
```

#### GET /package/&lt;string:package&gt;/README

Display README for package `package`.

Request:

```text
GET /package/foo/README HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: text/plain
```

Response:

```text
HTTP/1.1 200 OK
Content-Length: xxx
Content-Type: text/plain
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

# foo

Foo bar baz quz

...
```

#### GET /package/&lt;string:package&gt;/data

A tarball containing all data for `package`.

Request:

```text
GET /package/foo/data HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: application/tar+gzip
```

Response:

```text
HTTP/1.1 200 OK
Content-Length: xxx
Content-Type: application/tar+gzip
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Content-Disposition: attachment; filename="foo-1.0.1.tar.gz"
X-Integrity: 0123456789abcdef0123456789abcdef12345678
Date: Fri, 29 Aug 1997 22:14:00 GMT

<raw data>...
```

#### POST /package/publish

Publish a new package.

Note: This request must be authorized. See the section on
[Authorization](#authorization) to learn more.

Request:

```text
POST /package/publish HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: application/tar+gzip
Authorization: HMAC-SHA1 <base64 email:hmac-sha1>
Content-Length: xxx

<content>
```

Response:

```text
HTTP/1.1 200 OK
Content-Length: xxx
Content-Type: application/tar+gzip
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

{
    "title": "foo",
    "description": "Foo bar baz qux",
    "version": "1.0.1",
    "author": {
        "name": "Joe Somebody",
        "email": "joe.somebody@email.com"
    },
    "license": "GPLv3",
    "tags": [
        "foo",
        "bar",
        "baz"
    ]
}
```

If the package is published successfully, a `200` HTTP response code will be
returned along with the contents of the published `package.json`.

To learn more about the gzipped package content, see the section on
[Packages](/packages/#publishing).

### /user

#### POST /user/create

Create a user in the registry.

Request:

```text
POST /user/create HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: application/json
Content-Length: xxx

{
    "name": "<name>",
    "email": "<email>",
    "password": "<password>"
}
```

Response:

```text
HTTP/1.1 200 OK
Content-Length: xxx
Content-Type: application/json
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

{}
```

If the user already exists, the response will be:

```text
HTTP/1.1 409 CONFLICT
Content-Length: xxx
Content-Type: application/json
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

{
    "status": 409,
    "error": "User already exists"
}
```

#### POST /user/login

Log into the registry.

Request:

```text
POST /user/login HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: application/json
Authorization: Basic <base64 email:password>
Content-Length: xxx

```

Response:

```text
HTTP/1.1 200 OK
Content-Length: xxx
Content-Type: application/json
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

{
    "token": "<token>"
}
```

#### POST /user/logout

Log out of the registry.

Note: This request must be authorized. See the section on
[Authorization](#authorization) to learn more.

Request:

```text
POST /user/logout HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: application/json
Authorization: HMAC-SHA1 <base64 email:hmac-sha1>
Content-Length: xxx

{}
```

Response:

```text
HTTP/1.1 200 OK
Content-Length: xxx
Content-Type: application/json
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

{}
```

#### POST /user/forgotpassword

Send an email to reset password.

Request:

```text
POST /user/forgotpassword HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: application/json
Content-Length: xxx

{
    "email": "<email>"
}
```

Response:

```text
HTTP/1.1 200 OK
Content-Length: xxx
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

{}
```

#### POST /user/resetpassword

Reset password.

Note: Authorization in terms of generating the HMAC-SHA1 works just as described
in the [Authorization](#authorization) section, but the `token` used must be the
one emailed to the user after utilizing the
[`/user/forgotpassword`](#post-userforgotpassword) endpoint.

Request:

```text
POST /user/forgotpassword HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: application/json
Authorization: HMAC-SHA1 <base64 email:hmac-sha1>
Content-Length: xxx

{
    "email": "<email>",
    "password": "<password>"
}
```

Response:

```text
HTTP/1.1 200 OK
Content-Length: xxx
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

{}
```

### /reference

#### GET /reference

Display reference categories.

Request:

```text
GET /reference HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: text/plain
```

Response:

```text
HTTP/1.1 200 OK
Content-Length: xxx
Content-Type: text/plain
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

  foo
  bar
  baz
```

#### GET /reference/<path:path>

Display reference information for term `path`.

Request:

```text
GET /reference/foo/bar HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: text/plain
```

Response:

```text
HTTP/1.1 200 OK
Content-Length: xxx
Content-Type: text/plain
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

  baz
  qux
```

If no such path exists, the response will be:

```text
HTTP/1.1 404 NOT FOUND
Content-Length: xxx
Content-Type: text/plain
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

{
    "status": 404,
    "error": "Not Found"
}
```

## Authorization

Some requests must be authorized. Here is the workflow:

1. Get a `token` from the [`/user/login`](#post-userlogin) endpoint.
2. Generate a `HMAC-SHA1` using the `token` as the `key` and a concatenation
of the method and endpoint (with a `+`) as the `message`. (Ex:
`POST+/user/logout`)
3. Set the `Authorization` header to `HMAC-SHA1 <base64 email:hmac-sha1>` where
the user's email and the `HMAC-SHA1` are concatenated (with a `:`) and Base64
encoded.

Here is an example in Python:

```python
#!/usr/bin/env python
"""Authorization example"""

import base64
import hmac
from hashlib import sha1
import requests

# get token
login_request = requests.post('http://[DOMAIN]/user/login', auth=('[EMAIL]', '[PASSWORD]'))

if login_request.status_code != 200:
    print 'Login failed!'
    exit(1)

print 'Login successful!'

data = login_request.json()
token = data['token']

# generate HMAC-SHA1
hmac_hash = hmac.new(str(token), 'POST+/user/logout', sha1)
hmac_sha1 = hmac_hash.hexdigest()

# send authorized request
base64_auth = base64.b64encode('%s:%s' % ('[EMAIL]', hmac_sha1))
headers = {
    'Authorization': 'HMAC-SHA1 %s' % (base64_auth)
}
logout_request = requests.post('http://[DOMAIN]/user/logout', headers=headers)

if logout_request.status_code != 200:
    print 'Logout failed!'
    exit(1)

print 'Logout successful!'
```
