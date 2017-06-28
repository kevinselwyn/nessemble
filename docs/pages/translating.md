# Translating

`nessemble` is set up to be completely translated with the following steps:

## Prepare

A master language template must be generated first:

```text
make translate-template
```

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> This step only needs to be performed once, unless the version of `nessemble`
> increases.
> </div>

## Generate

To generate a specific language template:

```text
make translate-new LANG=<ll_CC>
```

A file named `translate/<ll_CC/nessemble.po` will be created.

`ll_CC` must be a valid locale where:

* `ll` - Valid language code
* `CC` - Valid country code

Example:

```text
make translate-new LANG=de_DE
```

`de_DE` is the locale for Germany.

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> If no language template exists, one will be created during this step.
> </div>

## Populate

Populate the file created during the previous step
(`translate/<ll_CC>/nessemble.po`).

Example:

```text
msgid "String to translate"
msgstr "String zu übersetzen"
```

## Compile

The language file must be compiled:

```text
make translate-compile LANG=<ll_CC>
```

## Install

To install locally:

```text
make translate-install LANG=<ll_CC>
```

The locale directory differs by platform:

| Platform  | Location                                    |
|-----------|---------------------------------------------|
| Linux/Mac | /home/&lt;username&gt;/.nessemble/locale/   |
| Windows   | \Users\&lt;username&gt;\\.nessemble\locale\ |

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> The previous compilation step will be performed automatically before
> installation.
> </div>
