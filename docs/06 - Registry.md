# Registry

Registries have no preferred language but must contain the following endpoints
to be valid.

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

### GET /

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
            "name": "foo",
            "description": "Foo bar baz qux"
        },
        ...
    ]
}
```
