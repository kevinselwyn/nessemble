# Registry

Registries have no preferred language but must contain the following endpoints
to be valid.

Note: To change the registry URL, run:

```
nessemble registry <URL>
```

## Endpoints

| Endpoint                               | Method   | Description                                   |
|----------------------------------------|----------|-----------------------------------------------|
| /                                      | GET      | List of all packages                          |
| /search/&lt;string:term&gt;            | GET      | Search for packages by `term`                 |
| /package/&lt;string:package&gt;        | GET      | Display information about package `package`   |
| /package/&lt;string:package&gt;/README | GET      | Display README for package `package`          |
| /package/&lt;string:package&gt;/data   | GET      | A tarball containing all data for `package`   |
| /user/create                           | POST     | Create a user in the registry                 |
| /user/login                            | POST     | Log into the registry                         |
| /user/logout                           | GET/POST | Log out of the registry                       |
| /reference                             | GET      | Display reference categories                  |
| /reference/<path:path>                 | GET      | Display reference information for term `path` |

### Root

#### GET /

List of all packages.

Request:

```
GET / HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: application/json
```

Response:

```
HTTP/1.0 200 OK
Content-Length: xxx
Content-Type: application/json
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

{
    "libraries": [
        {
            "url": "http://xxxxx.xxx/foo",
            "tags": [
                "foo",
                "bar",
                "baz"
            ],
            "title": "foo",
            "description": "Foo bar baz qux"
        },
        ...
    ]
}
```

### /search

#### GET /search/&lt;string:term&gt;

Search for packages by `term`.

Request:

```
GET /search/foo HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: application/json
```

Response:

```
HTTP/1.0 200 OK
Content-Length: xxx
Content-Type: application/json
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

{
    "libraries": [
        {
            "url": "http://xxxxx.xxx/foo",
            "tags": [
                "foo",
                "bar",
                "baz"
            ],
            "title": "foo",
            "description": "Foo bar baz qux"
        },
        ...
    ]
}
```

### /package

#### GET /package/&lt;string:package&gt;

Display information about package `package`.

Request:

```
GET /package/foo HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: application/json
```

Response:

```
HTTP/1.0 200 OK
Content-Length: xxx
Content-Type: application/json
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

{
    "resource": "http://xxxxx.xxx/foo/data",
    "title": "foo",
    "license": "GPLv3",
    "author": {
        "name": "Joe Somebody",,
        "email": "joe.somebody@email.com"
    },
    "tags": [
        "foo",
        "bar",
        "baz"
    ],
    "version": "1.0.1",
    "readme": "http://xxxxx.xxx/foo/README",
    "description": "Foo bar baz qux"
}
```

#### GET /package/&lt;string:package&gt;/README

Display README for package `package`.

Request:

```
GET /package/foo/README HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: text/plain
```

Response:

```
HTTP/1.0 200 OK
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

```
GET /package/foo/data HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: application/tar+gzip
```

Response:

```
HTTP/1.0 200 OK
Content-Length: xxx
Content-Type: application/tar+gzip
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

<raw data>...
```

### /user

#### POST /user/create

Create a user in the registry.

Request:

```
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

```
HTTP/1.0 200 OK
Content-Length: xxx
Content-Type: application/json
Access-Control-Allow-Origin: *
Server: Nessemble
X-Response-Time: x.xxx
Date: Fri, 29 Aug 1997 22:14:00 GMT

{}
```

If the user already exists, the response will be:

```
HTTP/1.0 409 CONFLICT
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

```
POST /user/login HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: application/json
Authorization: Basic <base64 email:password>
Content-Length: xxx

<email>:<password>
```

Response:

```
HTTP/1.0 200 OK
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

Request:

```
POST /user/logout HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: application/json
X-Auth-Token: <token>
Content-Length: xxx

{}
```

Response:

```
HTTP/1.0 200 OK
Content-Length: xxx
Content-Type: application/json
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

```
GET /reference HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: text/plain
```

Response:

```
HTTP/1.0 200 OK
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

```
GET /reference/foo/bar HTTP/1.1
Host: xxxxx
User-Agent: nessemble/1.0.1
Content-Type: text/plain
```

Response:

```
HTTP/1.0 200 OK
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

```
HTTP/1.0 404 NOT FOUND
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
