(function (window) {
    var Analytics = function () {
        this.events = [
            {
                selector: '.navbar-header a',
                category: 'Navigation',
                action: 'Page Top',
                label: 'Website'
            },
            {
                selector: '.navbar-collapse .hidden a',
                category: 'Navigation',
                action: 'Page Top',
                label: 'Website Mobile'
            },
            {
                selector: '.navbar-collapse a.page-scroll',
                category: 'Navigation',
                action: null,
                label: 'Website Mobile'
            },
            {
                selector: '.intro-text a',
                category: 'Intro Download',
                action: null,
                label: 'Website'
            },
            {
                selector: '.intro-body .page-scroll',
                category: 'Navigation',
                action: 'Read More',
                label: 'Website'
            },
            {
                selector: '#download .container a',
                category: 'Link',
                action: 'Documentation',
                label: 'Website'
            },
            {
                selector: '#download .list-inline a',
                category: 'Download',
                action: null,
                label: 'Website'
            },
            {
                selector: '#contribute p a',
                category: 'Contribute',
                action: 'GitHub Link',
                label: 'Website'
            },
            {
                selector: '#contribute .list-inline a',
                category: 'Contribute',
                action: 'GitHub Button',
                label: 'Website'
            },
            {
                selector: '#asciinema',
                category: 'Asciinema',
                action: 'Play',
                label: 'Website',
                event: 'play'
            },
            {
                selector: '#asciinema',
                category: 'Asciinema',
                action: 'Pause',
                label: 'Website',
                event: 'pause'
            }
        ];

        window.addEventListener('load', this.attachEvents.bind(this), false);
    };

    Analytics.prototype.attachEvents = function () {
        var events = this.events,
            els = null,
            ga = null,
            i = 0,
            j = 0,
            k = 0,
            l = 0;

        ga = window.ga || function () {};

        for (i = 0, j = events.length; i < j; i += 1) {
            els = document.querySelectorAll(events[i].selector);

            if (!els) {
                continue;
            }

            for (k = 0, l = els.length; k < l; k += 1) {
                els[k].addEventListener(events[i].event || 'click', (function (el, data) {
                    var action = data.action;

                    if (!action) {
                        action = el.innerText.replace(/^\s+/g, '').replace(/\s+$/g, '').toLowerCase();
                        action = action.charAt(0).toUpperCase() + action.slice(1);
                    }

                    return function () {
                        ga('send', 'event', data.category, action, data.label);
                    };
                }(els[k], events[i])));
            }
        }
    };

    var analytics = new Analytics();
}(window));
