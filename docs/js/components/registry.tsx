import * as React from 'react';
import * as autoBind from 'react-autobind';
import * as classNames from 'classnames';

import Registry from '../models/registry';
import TrackingActions from '../actions/tracking';
import TrackingStore from '../stores/tracking';

type RegistryMethod =
    'GET'
    | 'POST';

interface RegistryComponentProps {
    endpoint?: string;
    method?: RegistryMethod;
    registry?: string;
}

class RegistryComponent extends React.Component<RegistryComponentProps, any> {
    listenToTracking: any;

    constructor(props: RegistryComponentProps) {
        super(props);
        autoBind(this);

        this.state = {
            endpoint: props.endpoint,
            headers: '',
            headers_show: false,
            body: '',
            body_show: false
        };
    }

    componentDidMount() {
        var modal_title = document.querySelector('.modal-header'),
            modal_body = document.querySelector('.modal-body');

        if (modal_title) {
            modal_title.innerHTML = 'Registry Help';
        }

        if (modal_body) {
            modal_body.innerHTML = '<p>Click the <code>Send</code> button to run a specific API call.</p>';
        }

        this.listenToTracking = TrackingStore.listen(this._updateTracking, TrackingStore);
    }

    componentWillUnmount() {
        this.listenToTracking();
    }

    _updateTracking(props: any) {}

    _onChangeEndpoint(e: any) {
        this.setState({
            endpoint: e.target.value
        });
    }

    _onSend() {
        var reg: Registry = new Registry(this.props.registry, this.state.endpoint, this.props.method);

        reg.run((hdrs: string, res: string, xhr: any): void => {
            var new_state: any = {};

            if (hdrs.length) {
                new_state.headers = hdrs;
                new_state.headers_show = true;
            }

            if (res.length) {
                new_state.body = res;
                new_state.body_show = true;
            }

            this.setState(new_state);
        });

        TrackingActions.trackEvent('Registry', 'Send', 'Documentation', {
            dimension1: this.state.endpoint,
            dimension2: this.props.method
        });
    }

    _onKeydownEndpoint(e: any) {
        if (e.keyCode !== 13) {
            return;
        }

        e.preventDefault();

        this._onSend();
    }

    _onReset() {
        this.setState({
            endpoint: this.props.endpoint,
            headers: '',
            headers_show: false,
            body: '',
            body_show: false
        });

        TrackingActions.trackEvent('Registry', 'Reset', 'Documentation', {
            dimension1: this.props.endpoint
        });
    }

    _onClear() {
        this.setState({
            headers: '',
            headers_show: false,
            body: '',
            body_show: false
        })

        TrackingActions.trackEvent('Registry', 'Clear', 'Documentation');
    }

    _onOpen() {
        var url = [this.props.registry, this.state.endpoint].join('');

        TrackingActions.trackEvent('Registry', 'Open', 'Documentation');

        window.open(url, '_blank');
    }

    render() {
        return (
            <div>
                <hr />
                <p>Try it:</p>
                <textarea
                    className="registry"
                    disabled={true}
                    readOnly={true}
                    value={this.props.registry} />
                <textarea
                    className="endpoint"
                    value={this.state.endpoint}
                    onChange={this._onChangeEndpoint}
                    onKeyDown={this._onKeydownEndpoint} />
                <div
                    className="btn-group">
                    <button
                        className={classNames({
                            btn: true,
                            'btn-danger': true
                        })}
                        title="Send"
                        onClick={this._onSend}>Send</button>
                    <button
                        className={classNames({
                            btn: true,
                            'btn-neutral': true
                        })}
                        title="Reset"
                        onClick={this._onReset}>Reset</button>
                    <button
                        className={classNames({
                            btn: true,
                            'btn-neutral': true
                        })}
                        title="Clear"
                        onClick={this._onClear}>Clear</button>
                    {
                        this.props.method === 'GET' ? (
                            <button
                                className={classNames({
                                    btn: true,
                                    'btn-neutral': true
                                })}
                                title="Open in new tab"
                                onClick={this._onOpen}>
                                <i
                                    className={classNames({
                                        fa: true,
                                        'fa-external-link': true
                                    })} />
                            </button>
                        ) : null
                    }
                    <button
                        className={classNames({
                            btn: true,
                            'btn-neutral': true
                        })}
                        title="Help"
                        data-toggle="modal"
                        data-target="#help-modal">
                        <i
                            className={classNames({
                                fa: true,
                                'fa-question-circle': true
                            })} />
                    </button>
                </div>
                <pre>
                    <code
                        className={classNames({
                            text: true,
                            show: this.state.headers_show
                        })}>{this.state.headers}</code>
                    <code
                        className={classNames({
                            text: true,
                            show: this.state.body_show
                        })}>{this.state.body}</code>
                </pre>
            </div>
        );
    }
}

export default RegistryComponent;
