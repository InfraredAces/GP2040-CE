import React, { useEffect, useRef, useState } from 'react';
import { Button, Modal } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import WebApi from '../Services/WebApi';
import { KEY_CODES } from '../Data/Keyboard';

const CaptureKeycodeButton = ({
	labels,
	onChange,
	small = false,
	buttonLabel,
}) => {
	const { t } = useTranslation('');
	const [showModal, setShowModal] = useState(false);
	const [labelIndex, setLabelIndex] = useState(0);
	const [triggerCapture, setTriggerCapture] = useState(false);

	let data = '';
	let keycode = '';

	const currentLabel = labels[labelIndex] || '';
	const hasNext = Boolean(labels[labelIndex + 1]);

	const timeout = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

	const closeAndReset = () => {
		document.removeEventListener('keydown', (e) => {
			data = e.code;
			keycode = parseKeycode(data);
		});
		onChange(currentLabel, keycode);
		setTriggerCapture(false);
		setShowModal(false);
		setLabelIndex(0);
	};

	const getKeycode = async () => {
		setTriggerCapture(false);

		document.addEventListener(
			'keydown',
			(e) => {
				data = e.code;
				console.log(e.which);
				keycode = parseKeycode(data);
				return closeAndReset();
			},
			{ once: true },
		);

		setLabelIndex((index) => index + 1);
		setTriggerCapture(true);
	};

	function parseKeycode(keycode: string) {
		if (keycode.startsWith('Key')) {
			return keycode.substring(3);
		} else if (keycode.startsWith('Digit')) {
			return keycode.substring(5);
		} else if (keycode == ' ') {
			return 'Space';
		} else if (keycode == 'CapsLock') {
			return 'CapsLock';
		} else {
			return keycode.split(/(?=[A-Z])/).join(' ');
		}
	}

	const stopCapture = async () => {
		await timeout(50);
		return closeAndReset();
	};

	const skipButton = async () => {
		await timeout(50);
	};

	useEffect(() => {
		if (triggerCapture) {
			setShowModal(true);
			getKeycode();
		}
	}, [triggerCapture]);

	return (
		<>
			<Modal centered show={showModal} onHide={() => stopCapture()}>
				<Modal.Header closeButton>
					<Modal.Title className="me-auto">{`${t(
						'CaptureButton:capture-button-modal-title',
					)} ${currentLabel}`}</Modal.Title>
				</Modal.Header>
				<Modal.Body className="row">
					<span className="col-sm-10">
						{t('CaptureButton:capture-button-modal-content')}
					</span>
					<span className="col-sm-1">
						<span className="spinner-border" />
					</span>
				</Modal.Body>
				<Modal.Footer>
					{hasNext && (
						<Button variant="secondary" onClick={() => skipButton()}>
							{t('CaptureButton:capture-button-modal-skip')}
						</Button>
					)}
					<Button variant="danger" onClick={() => stopCapture()}>
						{t('CaptureButton:capture-button-modal-stop')}
					</Button>
				</Modal.Footer>
			</Modal>
			<Button variant="secondary" onClick={() => setTriggerCapture(true)}>
				{small
					? '🎮'
					: `${
							buttonLabel
								? buttonLabel
								: t('CaptureButton:capture-button-button-label')
					  } 🎮`}
			</Button>
		</>
	);
};

export default CaptureKeycodeButton;
