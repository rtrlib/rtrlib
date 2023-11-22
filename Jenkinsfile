pipeline {

	agent {
		dockerfile {
			filename 'gcc'
			dir 'scripts/dockerfiles'
		}
	}

	stages {
		stage('Build') {
			steps {
				sh 'scripts/travis.sh'
			}
		}
	}
}
